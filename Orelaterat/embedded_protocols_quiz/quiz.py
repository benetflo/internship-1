from quiz_data import quiz_questions
import random

from tkinter import *
from tkinter import ttk

class Quiz:

    amount_of_questions = 0
    all_quiz_questions = {}
    current_right_answ = ""
    question = {}
    score = 0

    def __init__(self, ext_quiz_questions):
        self.amount_of_questions = len(ext_quiz_questions)
        self.all_quiz_questions = ext_quiz_questions

    # SELECTS NEW RANDOM QUESTION
    def select_new_random_question(self):
        self.question = random.choice(quiz_questions)

        # shuffle the options before assigning right "choices" option for right answer
        random.shuffle(self.question["options"])

        for opt in self.question["options"]:
            if opt == self.question["answer"]:
                self.current_right_answ = opt
    # GET METHODS
    def get_question(self):
        return self.question["question"]
    
    def get_options(self):
        return self.question["options"]
    
    def get_answer(self):
        return self.question["answer"]

    # PRINT METHODS
    def print_question(self):
        print(self.get_question())

    def print_options(self):
        options = self.get_options()

        for opt, choice in zip(options, self.choices):
            print(f"{choice}: {opt}")

    def check_answer(self, option):
        if option == self.current_right_answ:
            self.score += 1
        else:
            self.score -= 1


root = Tk()

frm = ttk.Frame(root, padding=30)
frm.grid()

q = Quiz(quiz_questions)
q.select_new_random_question()

question_label = ttk.Label(frm, text="", font=("Arial", 12))
question_label.grid(column=0, row=0, columnspan=2, pady=(0, 10))

buttons = []

score_label = ttk.Label(frm, text=f"Score: {q.score}")
score_label.grid(column=0, row=6, pady=(20,0), sticky="ew")


def answer_clicked(selected_option):
    q.check_answer(selected_option)  
    score_label.config(text=f"Score: {q.score}")  
    q.select_new_random_question()  
    show_question() 

def show_question():
    question_label.config(text=q.get_question())

    # Remove old buttons
    for btn in buttons:
        btn.destroy()
    buttons.clear()

    # Create new buttons
    for i, option in enumerate(q.get_options()):
        btn = ttk.Button(frm, text=option, command=lambda opt=option: answer_clicked(opt))
        btn.grid(column=0, row=i+1, sticky="ew", pady=5)
        buttons.append(btn)

ttk.Button(frm, text="Quit", command=root.destroy).grid(column=0, row=5, pady=(20,0), sticky="ew")

show_question()

root.mainloop()
