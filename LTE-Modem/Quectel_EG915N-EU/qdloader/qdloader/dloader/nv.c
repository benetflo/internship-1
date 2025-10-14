/******************************************************************************
  @file    nv.c
  @brief   nv.

  DESCRIPTION
  QDloader Tool for USB and PCIE of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2021 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "../std_c.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include "pac.h"
#include "j_queue.h"

#define MAX_NV_CORE_NAME (16)
#define INDEX_NV_ID (0x0)
#define PADDING_NV_ID (0xFFFE)

#define MAX_NV 1024
#define MAX_PADDING_SIZE (512)

#define INDEX_MAGIC (0x58444E49) // INDX
#define INDEX_HEADER_VER1 (0x0)

typedef struct _MULTI_CORE_NV_HEADER_T_
{
    uint32_t dwMagic;
    uint16_t wVersion;
    uint16_t wCountCore; // the count of nv core
} MULTI_CORE_NV_HEADER_T, *PMULTI_CORE_NV_HEADER_PTR;

typedef struct _NV_CORE_INFO_T_
{
    char szCoreName[MAX_NV_CORE_NAME]; // nv core name,
    uint32_t dwDataOffset;             // the offset of nv data
    uint32_t dwDataSize;               // the length of nv data
} NV_CORE_INFO_T, *PNV_CORE_INFO_PTR;

struct nv_item
{
    uint16_t wCurID;
    uint16_t dwLength;
    uint32_t dwOffset;
};

static struct nv_item src_list[MAX_NV];
static struct nv_item dst_list[MAX_NV];
static uint16_t *backup_list;
static size_t backup_num;
#define GSM_IMEI_ITEM_ID (0x5)

static int needBackup(uint16_t id)
{
    size_t i;

    if (id == GSM_IMEI_ITEM_ID) return 1;

    if (!backup_list || !backup_num) return 0;

    for (i = 0; i < backup_num; i++)
    {
        if (id == backup_list[i]) return 1;
    }

    return 0;
}

static uint32_t build_nv_list(uint8_t *lpCode, uint32_t dwCodeSize, struct nv_item *nv_list, int bCoreNV)
{
    uint16_t bRet;
    uint16_t dwLength;
    uint32_t dwOffset;
    uint8_t *pTemp;
    uint32_t i = 0;

    dwOffset = bCoreNV ? 0 : 4; // Skip first four bytes,that is time stamp
    pTemp = lpCode + dwOffset;

    while (dwOffset < dwCodeSize && i++ < MAX_NV)
    {
        nv_list->dwOffset = dwOffset;

        nv_list->wCurID = le16toh(*(uint16_t *)pTemp);
        pTemp += 2;

        nv_list->dwLength = dwLength = le16toh(*(uint16_t *)pTemp);

        pTemp += 2;

        // printf("{%04x, %04x}\n", nv_list->wCurID, nv_list->dwLength);

        dwOffset += 4;

        // Must be four byte aligned
        bRet = (dwLength % 4);
        if (bRet != 0) dwLength += 4 - bRet;

        if (nv_list->wCurID == 0xFFFF || nv_list->wCurID == 0xFFFE)
        {
            // printf("%s Get %x\n", __func__, nv_list->wCurID);
            return i;
        }

        if (dwOffset == dwCodeSize)
        {
            // printf("%s Get EOL\n", __func__);
            return i;
        }

        nv_list++;
        dwOffset += dwLength;
        pTemp += dwLength;
    }

    return 0;
}

int lowMemorySendData(uint8_t *send_buffer, size_t size, pac_ctx_t *ctx, Scheme_t *File, int flag)
{
    fwrite(send_buffer, 1, size, out_fp);
    return 0;
}

int IsMultiCoreLowMemory(const uint8_t *lpBuf, uint32_t dwSize, MULTI_CORE_NV_HEADER_T *pCoreHeader)
{
    int bMultiCore = 0;

    if (lpBuf && (dwSize >= 8 + sizeof(MULTI_CORE_NV_HEADER_T)))
    {
        uint16_t wID = *(uint16_t *)(lpBuf + 4);
        uint16_t wLen = *(uint16_t *)(lpBuf + 6);
        uint16_t wLen_tmp = 0;
        wLen_tmp = wLen;

        wLen_tmp = ((le16toh(wLen_tmp) + 3) / 4) * 4;
        if (INDEX_NV_ID == le16toh(wID) && le16toh(wLen_tmp) > sizeof(MULTI_CORE_NV_HEADER_T))
        {
            memcpy(pCoreHeader, lpBuf + 8, sizeof(MULTI_CORE_NV_HEADER_T));
            if (INDEX_MAGIC == le32toh(pCoreHeader->dwMagic))
            {
                if (INDEX_HEADER_VER1 == le16toh(pCoreHeader->wVersion))
                {
                    bMultiCore = 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    return bMultiCore;
}

static int initSrcNvList(FILE *lpCodeSrc, uint32_t dwCodeSizeSrc, int bCoreNV, J_Queue *src_nv_list_que)
{
    uint16_t bRet;
    uint32_t dwOffset = bCoreNV ? 0 : 4;
    uint16_t dwLength = 0;
    if (dwOffset > 0) fseek(lpCodeSrc, dwOffset, SEEK_CUR);
    while (dwOffset < dwCodeSizeSrc)
    {
        Nv_Item src_item;
        if (4 != fread(g_nv_src_buffer, 1, 4, lpCodeSrc))
        {
            dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
        }
        src_item.dwOffset = dwOffset;
        src_item.wCurID = le16toh(*(uint16_t *)g_nv_src_buffer);
        src_item.dwLength = dwLength = le16toh(*(uint16_t *)(g_nv_src_buffer + 2));

        if (verbose > 2) dprintf("src: [%d] [%d] [%d]\r\n", src_item.wCurID, src_item.dwLength, src_item.dwOffset);

        if (src_item.wCurID > 0xFFFE)
        {
            break;
        }
        else if (src_item.wCurID > 0)
        {
            enqueue(src_nv_list_que, src_item);
        }

        bRet = (dwLength % 4);
        if (bRet != 0) dwLength += 4 - bRet;

        dwOffset += 4;
        dwOffset += dwLength;
        fseek(lpCodeSrc, dwLength, SEEK_CUR);
    }
    return 0;
}

int mergeNormalNvLowMemory(FILE *lpCodeSrc, uint32_t dwCodeSizeSrc, long src_offset_backup, FILE *lpCodeDest, uint32_t dwCodeSizeDest, long dst_offset_backup, int bCoreNV,
                           pac_ctx_t *ctx, Scheme_t *File)
{
    J_Queue src_nv_list_que;
    initQueue(&src_nv_list_que);

    initSrcNvList(lpCodeSrc, dwCodeSizeSrc, bCoreNV, &src_nv_list_que);

    if (isQueueEmpty(&src_nv_list_que))
    {
        if (verbose) dprintf("branch 02 and length [%d]\r\n", dwCodeSizeDest);
        if (dwCodeSizeDest != fread(g_nv_dst_buffer, 1, dwCodeSizeDest, lpCodeDest))
        {
            dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
        }
        lowMemorySendData(g_nv_dst_buffer, dwCodeSizeDest, ctx, File, 0);
        return 1;
    }

    uint32_t dwOffset = bCoreNV ? 0 : 4;
    uint16_t dwLength = 0;
    uint16_t bRet;
    if (dwOffset > 0)
    {
        fseek(lpCodeDest, dwOffset, SEEK_CUR);
    }

    uint32_t read_dst_size = 0;
    while (dwOffset < dwCodeSizeDest)
    {
        read_dst_size = min(BURN_MAX_SIZE, dwCodeSizeDest - dwOffset);
        memset(g_nv_dst_buffer, 0, sizeof(g_nv_dst_buffer));

        fseek(lpCodeDest, dst_offset_backup + dwOffset, SEEK_SET);
        uint32_t ans = fread(g_nv_dst_buffer, 1, read_dst_size, lpCodeDest);
        if (read_dst_size != ans)
        {
            dprintf("dst read buff size error [%d] [%d] [%s]\r\n", ans, errno, strerror(errno));
        }

        uint8_t *dst_p = NULL;
        uint32_t p = 0;
        for (p = 0; p < read_dst_size;)
        {
            dst_p = &g_nv_dst_buffer[p];
            Nv_Item dst_item;
            dst_item.dwOffset = dwOffset;
            dst_item.wCurID = le16toh(*(uint16_t *)dst_p);
            dst_item.dwLength = dwLength = le16toh(*(uint16_t *)(dst_p + 2));

            if (verbose) dprintf("dst: [%d] [%d] [%d] \n", dst_item.wCurID, dst_item.dwLength, dst_item.dwOffset);

            if (p + dwLength > read_dst_size)
            {
                break;
            }

            if (needBackup(dst_item.wCurID))
            {
                Nv_Item check_backup_item;
                if (j_query(&src_nv_list_que, dst_item.wCurID, &check_backup_item))
                {
                    if (verbose)
                        dprintf("need backup [%d] <<<--- [%d] [%d] [%d] \n", dst_item.wCurID, check_backup_item.wCurID, check_backup_item.dwLength, check_backup_item.dwOffset);

                    fseek(lpCodeSrc, src_offset_backup + check_backup_item.dwOffset, SEEK_SET);

                    if (4 != fread(g_nv_src_buffer, 1, 4, lpCodeSrc))
                    {
                        dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
                    }

                    uint16_t query_id = le16toh(*(uint16_t *)g_nv_src_buffer);
                    uint16_t query_lenth = le16toh(*(uint16_t *)(g_nv_src_buffer + 2));

                    if (query_id == dst_item.wCurID)
                    {
                        if (query_lenth != fread(g_nv_src_buffer, 1, query_lenth, lpCodeSrc))
                        {
                            dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
                        }
                        memcpy(dst_p + 4, g_nv_src_buffer, query_lenth);
                    }
                }
            }

            bRet = (dwLength % 4);
            if (bRet != 0) dwLength += 4 - bRet;
            dwOffset += 4;
            dwOffset += dwLength;
            p += (dwLength + 4);
        }
        lowMemorySendData(g_nv_dst_buffer, p, ctx, File, 0);
    }
    return 1;
}

int MergeNormalNV(uint8_t *lpCodeSrc, uint32_t dwCodeSizeSrc, uint8_t *lpCodeDest, uint32_t dwCodeSizeDest, int bCoreNV)
{
    int s, d;
    int src_c, dst_c;

    src_c = build_nv_list(lpCodeSrc, dwCodeSizeSrc, src_list, bCoreNV);
    if (!src_c)
    {
        return 0;
    }

    dst_c = build_nv_list(lpCodeDest, dwCodeSizeDest, dst_list, bCoreNV);
    if (!dst_c)
    {
        return 0;
    }

    for (s = 0; s < src_c; s++)
    {
        uint16_t wCurID = src_list[s].wCurID;

        if (!needBackup(wCurID)) continue;

        for (d = 0; d < dst_c; d++)
        {
            if (wCurID == dst_list[d].wCurID)
            {
                break;
            }
        }

        if (d == dst_c)
        {
            printf("src_list[%d] {%04x, %04x} not in dst!\n", s, src_list[s].wCurID, src_list[s].dwLength);
            continue;
        }

        if (wCurID == 0xFFFF || wCurID == 0xFFFE)
        {
            printf("%s Get %x\n", __func__, wCurID);
        }
        else
        {
            src_list[s].dwLength = dst_list[d].dwLength; // modify by Aaron 2022/05/19
            if (src_list[s].dwLength != dst_list[d].dwLength)
            {
                printf("src_list[%d] {%04x, %04x} not match dst_list[%d] {%04x, %04x}\n", s, src_list[s].wCurID, src_list[s].dwLength, d, dst_list[d].wCurID, dst_list[d].dwLength);
                return 0;
            }
        }

        // printf("%d - > %d\n", s, d);
        // from neil, dwLength do not include ' id (2 bytes) + length (2 bytes) '
        memcpy(lpCodeDest + dst_list[d].dwOffset + 4, lpCodeSrc + src_list[s].dwOffset + 4, src_list[s].dwLength);
    }

    return 1;
}

int IsMultiCore(const uint8_t *lpBuf, uint32_t dwSize, PMULTI_CORE_NV_HEADER_PTR *pCoreHeader, PNV_CORE_INFO_PTR *pCoreItem)
{
    int bMultiCore = 0;
    *pCoreHeader = NULL;
    *pCoreItem = NULL;

    if (lpBuf && (dwSize > 8 + sizeof(MULTI_CORE_NV_HEADER_T)))
    {
        uint16_t wID = *(uint16_t *)(lpBuf + 4);
        uint16_t wLen = *(uint16_t *)(lpBuf + 6);
        uint16_t wLen_tmp = 0;
        wLen_tmp = wLen;

        wLen_tmp = ((le16toh(wLen_tmp) + 3) / 4) * 4;
        if (INDEX_NV_ID == le16toh(wID) && le16toh(wLen_tmp) > sizeof(MULTI_CORE_NV_HEADER_T))
        {
            *pCoreHeader = (MULTI_CORE_NV_HEADER_T *)(lpBuf + 8);
            *pCoreItem = (NV_CORE_INFO_T *)(lpBuf + 8 + sizeof(MULTI_CORE_NV_HEADER_T));
            if (INDEX_MAGIC == le32toh((*pCoreHeader)->dwMagic))
            {
                if (INDEX_HEADER_VER1 == le16toh((*pCoreHeader)->wVersion))
                {
                    bMultiCore = 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    return bMultiCore;
}

int mergeNvLowMemory(FILE *src_fp, uint32_t dwCodeSizeSrc, pac_ctx_t *ctx, Scheme_t *File)
{
    FILE *dst_fp = ctx->pac_fp;
    int bMultiCoreSrc = 0;
    int bMultiCoreDest = 0;
    MULTI_CORE_NV_HEADER_T pCoreHeaderSrc;
    NV_CORE_INFO_T pCoreItemSrc[5];
    MULTI_CORE_NV_HEADER_T pCoreHeaderDest;
    NV_CORE_INFO_T pCoreItemDest[5];
    uint32_t i;
    uint32_t read_size = -1;
    uint32_t read_size_2 = -1;

    memset(g_nv_src_buffer, 0, sizeof(g_nv_src_buffer));
    memset(g_nv_dst_buffer, 0, sizeof(g_nv_dst_buffer));

    read_size = 8 + sizeof(MULTI_CORE_NV_HEADER_T);

    if (read_size != fread(g_nv_src_buffer, 1, read_size, src_fp))
    {
        dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
    }
    if (read_size != fread(g_nv_dst_buffer, 1, read_size, dst_fp))
    {
        dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
    }

    lowMemorySendData(g_nv_dst_buffer, read_size, ctx, File, 0);

    bMultiCoreSrc = IsMultiCoreLowMemory(g_nv_src_buffer, read_size, &pCoreHeaderSrc);
    bMultiCoreDest = IsMultiCoreLowMemory(g_nv_dst_buffer, read_size, &pCoreHeaderDest);

    read_size_2 = pCoreHeaderSrc.wCountCore * sizeof(NV_CORE_INFO_T);

    if (read_size_2 != fread(g_nv_src_buffer, 1, read_size_2, src_fp))
    {
        dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
    }
    memcpy(pCoreItemSrc, g_nv_src_buffer, read_size_2);

    if (read_size_2 != fread(g_nv_dst_buffer, 1, read_size_2, dst_fp))
    {
        dprintf("fread error [%d][%s]\r\n", errno, strerror(errno));
    }
    memcpy(pCoreItemDest, g_nv_dst_buffer, read_size_2);

    lowMemorySendData(g_nv_dst_buffer, read_size_2, ctx, File, 0);

    if (verbose)
    {
        dprintf("[src] nv head message: [%d] [%d] [%d] [%d]\r\n", bMultiCoreSrc, pCoreHeaderSrc.dwMagic, pCoreHeaderSrc.wCountCore, pCoreHeaderSrc.wVersion);
    }

    if (bMultiCoreSrc != bMultiCoreDest)
    {
        dprintf("Mismatched bMultiCore\n");
        return -1;
    }

    if (bMultiCoreSrc)
    {
        if (le16toh(pCoreHeaderSrc.wCountCore) != le16toh(pCoreHeaderDest.wCountCore))
        {
            dprintf("Mismatched wCountCore\n");
            return -1;
        }

        for (i = 0; i < le16toh(pCoreHeaderSrc.wCountCore); i++)
        {
            NV_CORE_INFO_T *pItemSrc = &pCoreItemSrc[i];
            NV_CORE_INFO_T *pItemDest = &pCoreItemDest[i];

            if (verbose)
            {
                dprintf("{%s, %x, %d}\n", pItemSrc->szCoreName, le32toh(pItemSrc->dwDataOffset), le32toh(pItemSrc->dwDataSize));
                dprintf("{%s, %x, %d}\n", pItemDest->szCoreName, le32toh(pItemDest->dwDataOffset), le32toh(pItemDest->dwDataSize));
            }

            if ((le32toh(pItemSrc->dwDataOffset) + le32toh(pItemSrc->dwDataSize) <= dwCodeSizeSrc) &&
                (le32toh(pItemDest->dwDataOffset) + le32toh(pItemDest->dwDataSize) <= File->image->dwLoFileSize))
            {
                long src_offset_backup = ftell(src_fp);
                long dst_offset_backup = ftell(dst_fp);

                if (verbose)
                {
                    dprintf("src :[%ld]\r\n", src_offset_backup);
                    dprintf("dst :[%ld]\r\n", dst_offset_backup);
                }

                if (!mergeNormalNvLowMemory(src_fp, le32toh(pItemSrc->dwDataSize), src_offset_backup, dst_fp, le32toh(pItemDest->dwDataSize), dst_offset_backup, 1, ctx, File))
                {
                    dprintf("MergeNormalNV fail.\n");
                    return -1;
                }
            }
            else
            {
                dprintf("Invalid core nv image file.\n");
                return -1;
            }
        }
    }
    else
    {
        if (!mergeNormalNvLowMemory(src_fp, dwCodeSizeSrc, 0, dst_fp, File->image->dwLoFileSize, 0, 0, ctx, File))
        {
            dprintf("MergeNormalNV fail.\n");
            return -1;
        }
    }

    // for (int i = 0; i < 8; i++)
    // {
    //     g_nv_dst_buffer[i] = 0xff;
    // }

    // lowMemorySendData(g_nv_dst_buffer, 8, ctx, File, 1);

    return 0;
}

int MergeNV(uint8_t *lpCodeSrc, uint32_t dwCodeSizeSrc, uint8_t *lpCodeDest, uint32_t dwCodeSizeDest)
{
    int bMultiCoreSrc = 0;
    int bMultiCoreDest = 0;
    MULTI_CORE_NV_HEADER_T *pCoreHeaderSrc = NULL;
    NV_CORE_INFO_T *pCoreItemSrc = NULL;
    MULTI_CORE_NV_HEADER_T *pCoreHeaderDest = NULL;
    NV_CORE_INFO_T *pCoreItemDest = NULL;
    uint32_t i;

    bMultiCoreSrc = IsMultiCore(lpCodeSrc, dwCodeSizeSrc, &pCoreHeaderSrc, &pCoreItemSrc);
    bMultiCoreDest = IsMultiCore(lpCodeDest, dwCodeSizeDest, &pCoreHeaderDest, &pCoreItemDest);

    if (bMultiCoreSrc != bMultiCoreDest)
    {
        printf("Mismatched bMultiCore\n");
        return -1;
    }

    if (bMultiCoreSrc)
    {
        if (le16toh(pCoreHeaderSrc->wCountCore) != le16toh(pCoreHeaderDest->wCountCore))
        {
            printf("Mismatched wCountCore\n");
            return -1;
        }

        for (i = 0; i < le16toh(pCoreHeaderSrc->wCountCore); i++)
        {
            NV_CORE_INFO_T *pItemSrc = pCoreItemSrc + i;
            NV_CORE_INFO_T *pItemDest = pCoreItemDest + i;

            printf("{%s, %x, %d}\n", pItemSrc->szCoreName, le32toh(pItemSrc->dwDataOffset), le32toh(pItemSrc->dwDataSize));

            if ((le32toh(pItemSrc->dwDataOffset) + le32toh(pItemSrc->dwDataSize) <= dwCodeSizeSrc) &&
                (le32toh(pItemDest->dwDataOffset) + le32toh(pItemDest->dwDataSize) <= dwCodeSizeDest))
            {
                if (!MergeNormalNV(lpCodeSrc + le32toh(pItemSrc->dwDataOffset), le32toh(pItemSrc->dwDataSize), lpCodeDest + le32toh(pItemDest->dwDataOffset),
                                   le32toh(pItemDest->dwDataSize), 1))
                {
                    printf("MergeNormalNV fail.\n");
                    return -1;
                }
            }
            else
            {
                printf("Invalid core nv image file.\n");
                return -1;
            }
        }
    }
    else
    {
        if (!MergeNormalNV(lpCodeSrc, dwCodeSizeSrc, lpCodeDest, dwCodeSizeDest, 0))
        {
            printf("MergeNormalNV fail.\n");
            return -1;
        }
    }

    return 0;
}

size_t nv_get_size(uint8_t *lpCodeSrc, uint32_t dwCodeSizeSrc)
{
    MULTI_CORE_NV_HEADER_T *pCoreHeader = NULL;
    NV_CORE_INFO_T *pCoreItem = NULL;
    int bMultiCore = 0;
    size_t maxSize = 0;

    bMultiCore = IsMultiCore(lpCodeSrc, dwCodeSizeSrc, &pCoreHeader, &pCoreItem);
    if (!bMultiCore)
    {
        // printf("only support bMultiCore!\n");
        return 0;
    }
    else
    {
        uint32_t i;

        for (i = 0; i < le16toh(pCoreHeader->wCountCore); i++, pCoreItem++)
        {
            size_t tmpSize = le32toh(pCoreItem->dwDataOffset) + le32toh(pCoreItem->dwDataSize);

            if (tmpSize > maxSize) maxSize = tmpSize;
        }
    }

    // maxSize = le32toh(maxSize);

    return maxSize;
}

#define goto_out(x)                                                                                                         \
    do                                                                                                                      \
    {                                                                                                                       \
        if (x)                                                                                                              \
        {                                                                                                                   \
            dprintf("failed: %s (%s: %s: %d). errno: %d (%s)\n", #x, __FILE__, __func__, __LINE__, errno, strerror(errno)); \
            goto out;                                                                                                       \
        }                                                                                                                   \
    } while (0);

int nvMergeFileLowMemory(const char *src_file, pac_ctx_t *ctx, Scheme_t *File)
{
    FILE *src_fp = NULL;
    long src_size = 0;
    int ret = -1;

    backup_list = ctx->NVBackupList;
    backup_num = ctx->NVBackupNum;

    if (!src_file) return ret;

    src_fp = fopen(src_file, "rb");
    goto_out(!src_fp);

    fseek(src_fp, 0, SEEK_END);
    src_size = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    ret = mergeNvLowMemory(src_fp, src_size, ctx, File);
    goto_out(ret != 0);

out:
    if (src_fp) fclose(src_fp);

    return ret;
}

int nv_merg_file(const char *src_file, uint8_t *dst_p, size_t dst_size, uint16_t *back_list, size_t back_num)
{
    FILE *src_fp = NULL;
    uint8_t *src_p = NULL;
    long src_size = 0;
    int ret = -1;

    backup_list = back_list;
    backup_num = back_num;

    if (!src_file) return ret;

    src_fp = fopen(src_file, "rb");
    goto_out(!src_fp);

    fseek(src_fp, 0, SEEK_END);
    src_size = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    src_p = (uint8_t *)mmap(NULL, src_size, PROT_READ, MAP_SHARED, fileno(src_fp), 0);
    goto_out(MAP_FAILED == src_p);

    ret = MergeNV(src_p, src_size, dst_p, dst_size);
    goto_out(ret != 0);

out:
    if (src_p && src_p != MAP_FAILED) munmap(src_p, src_size);
    if (src_fp) fclose(src_fp);

    return ret;
}

#ifdef NV_MAIN
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_fp = NULL;
char log_buf[1024];
const char *get_time(void) { return ""; }
static const char *nv_dir = "/tmp/nv_files";
static void print_nv(uint8_t *lpCode, struct nv_item *nv_list, uint32_t nv_cnt)
{
    uint32_t j;
    char nv_file[128];
    FILE *nv_fp;

    for (j = 0; j < nv_cnt; j++)
    {
        struct nv_item *pNv = (struct nv_item *)(lpCode + nv_list[j].dwOffset);

        // printf("{%x, %x}\n", nv_list[j].wCurID, nv_list[j].dwLength);
        snprintf(nv_file, sizeof(nv_file), "%s/%x", nv_dir, nv_list[j].wCurID);
        printf(" -> %s\n", nv_file);
        nv_fp = fopen(nv_file, "wb");
        if (nv_fp)
        {
            fwrite(pNv, 1, 4 + pNv->dwLength, nv_fp);
            fclose(nv_fp);
        }
        else
            printf("failed: fopen(%s). errno: %d (%s)\n", nv_file, errno, strerror(errno));
    }
}

int main(int argc, char *argv[])
{
    FILE *src_fp = NULL;
    uint8_t *src_p = NULL;
    long src_size = 0;
    int ret = -1;
    int bMultiCore = 0;
    MULTI_CORE_NV_HEADER_T *pCoreHeader = NULL;
    NV_CORE_INFO_T *pCoreItem = NULL;
    uint32_t i;
    const char *nv_file = "nr_fixnv1";
    uint32_t src_c;
    char rm_cmd[128];

    if (argc == 2) nv_file = argv[1];

    printf("extract '%s' to dir '%s'\n", nv_file, nv_dir);

    if (access(nv_dir, W_OK)) mkdir(nv_dir, 0644);
    if (access(nv_dir, W_OK)) printf("failed: access(%s). errno: %d (%s)\n", nv_dir, errno, strerror(errno));
    goto_out(access(nv_dir, W_OK));
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s/*", nv_dir);
    system(rm_cmd);

    src_fp = fopen(nv_file, "rb");
    if (!src_fp) printf("failed: fopen(%s). errno: %d (%s)\n", nv_file, errno, strerror(errno));
    goto_out(!src_fp);

    fseek(src_fp, 0, SEEK_END);
    src_size = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    src_p = (uint8_t *)mmap(NULL, src_size, PROT_READ, MAP_SHARED, fileno(src_fp), 0);
    goto_out(MAP_FAILED == src_p);

    bMultiCore = IsMultiCore(src_p, src_size, &pCoreHeader, &pCoreItem);

    if (bMultiCore)
    {
        for (i = 0; i < pCoreHeader->wCountCore; i++)
        {
            NV_CORE_INFO_T *pItem = pCoreItem + i;

            printf("{%s, %x, %d}\n", pItem->szCoreName, pItem->dwDataOffset, pItem->dwDataSize);

            if (pItem->dwDataOffset + pItem->dwDataSize <= src_size)
            {
                src_c = build_nv_list(src_p + pItem->dwDataOffset, pItem->dwDataSize, src_list, 1);
                if (src_c) print_nv(src_p + pItem->dwDataOffset, src_list, src_c);
            }
            else
            {
                printf("Invalid core nv image file.\n");
                goto out;
                ;
            }
        }
    }
    else
    {
        src_c = build_nv_list(src_p, src_size, src_list, 0);
        if (src_c) print_nv(src_p, src_list, src_c);
    }

out:
    if (src_p && src_p != MAP_FAILED) munmap(src_p, src_size);
    if (src_fp) fclose(src_fp);

    return ret;
}
#endif
