import random
import tkinter as tk
from tkinter import messagebox, ttk


def process_words():
    raw_text = input_text.get("1.0", tk.END).strip()

    # 안내 문구만 있거나 비어있는 경우 예외 처리
    if not raw_text or raw_text.startswith("여기에 단어를 입력하세요"):
        messagebox.showwarning("알림", "섞을 단어를 입력창에 적어주세요.")
        return

    # 줄바꿈, 쉼표, 공백 기준 분리
    words = [
        w.strip()
        for w in raw_text.replace("\n", " ").replace(",", " ").split(" ")
        if w.strip()
    ]

    if not words:
        messagebox.showwarning("알림", "유효한 단어가 없습니다.")
        return

    random.shuffle(words)

    try:
        cols = int(col_entry.get())
        if cols < 1:
            cols = 1
    except ValueError:
        cols = 1

    # 한글 표 형태(\t, \n) 구성
    rows = []
    for i in range(0, len(words), cols):
        row_words = words[i : i + cols]
        rows.append("\t".join(row_words))

    result_str = "\n".join(rows)

    output_text.config(state="normal")
    output_text.delete("1.0", tk.END)
    output_text.insert(tk.END, result_str)
    output_text.config(state="disabled")

    root.clipboard_clear()
    root.clipboard_append(result_str)
    messagebox.showinfo(
        "완료",
        "단어가 무작위로 섞여 클립보드에 복사되었습니다!\n한글 표 드래그 후 Ctrl+V 하세요.",
    )


def clear_placeholder(event):
    if input_text.get("1.0", tk.END).strip().startswith(
        "여기에 단어를 입력하세요"
    ):
        input_text.delete("1.0", tk.END)
        input_text.config(fg="black")


# UI 구성
root = tk.Tk()
root.title("한글 표 전용 단어 셔플러")
root.geometry("520x620")
root.configure(bg="#F5F5F5")

# 1. 입력 영역 (상단)
frame_input = tk.LabelFrame(
    root,
    text=" 1. 단어 작성 칸 (여기에 적으세요) ",
    font=("맑은 고딕", 11, "bold"),
    bg="#F5F5F5",
    padx=10,
    pady=10,
)
frame_input.pack(fill="both", expand=True, padx=15, pady=(15, 5))

input_text = tk.Text(
    frame_input,
    height=8,
    font=("맑은 고딕", 10),
    bg="white",
    fg="gray",
    bd=1,
    relief="solid",
)
input_text.insert(
    "1.0",
    "여기에 단어를 입력하세요.\n예시: 사과, 바나나, 포도, 딸기, 수박 (공백, 쉼표, 줄바꿈 모두 가능)",
)
input_text.bind("<FocusIn>", clear_placeholder)
input_text.pack(fill="both", expand=True)

# 2. 옵션 영역 (중간)
frame_opt = tk.Frame(root, bg="#F5F5F5")
frame_opt.pack(fill="x", padx=15, pady=10)

tk.Label(
    frame_opt,
    text="한글 표 가로(열) 칸 수:",
    font=("맑은 고딕", 10, "bold"),
    bg="#F5F5F5",
).pack(side="left")
col_entry = tk.Entry(
    frame_opt,
    width=5,
    font=("맑은 고딕", 10, "bold"),
    justify="center",
    bd=1,
    relief="solid",
)
col_entry.insert(0, "1")
col_entry.pack(side="left", padx=10)

# 3. 실행 버튼
btn_shuffle = tk.Button(
    root,
    text="🎲 단어 무작위 섞기 & 클립보드 복사",
    command=process_words,
    bg="#2196F3",
    fg="white",
    font=("맑은 고딕", 11, "bold"),
    height=2,
    cursor="hand2",
    bd=0,
)
btn_shuffle.pack(fill="x", padx=15, pady=5)

# 4. 결과 출력 영역 (하단)
frame_output = tk.LabelFrame(
    root,
    text=" 2. 섞인 결과 (자동 복사됨) ",
    font=("맑은 고딕", 11, "bold"),
    bg="#F5F5F5",
    padx=10,
    pady=10,
)
frame_output.pack(fill="both", expand=True, padx=15, pady=(5, 15))

output_text = tk.Text(
    frame_output,
    height=8,
    font=("맑은 고딕", 10),
    bg="#E0E0E0",
    bd=1,
    relief="solid",
    state="disabled",
)
output_text.pack(fill="both", expand=True)

root.mainloop()