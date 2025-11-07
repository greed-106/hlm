import os
from os import path
import numpy as np
from matplotlib import pyplot as plt
import tkinter as tk
from tkinter.filedialog import askdirectory

params = [
    ("slice_id", "i"),("mb_y", "i"), ("mb_x", "i"), ("pred_mode", "i"), ("stad", "i"), ("mb_complex", "i"), ("luma_complex", "i"), ("chroma_complex", "i"), ("block_cost", "i"), ("fullness", "i"), ("QpY", "i"), ("QpU", "i"), ("QpV", "i")
]
transformers = [
    lambda x: x,  # slice_id
    lambda x: x,  # mb_y
    lambda x: x,  # mb_x
    lambda x: x,  # pred_mode (0:intra4x4 1:intra16x8 2:ibc)
    lambda x: x,  # satd
    lambda x: x,  # mb complexity
    lambda x: x,  # luma complexity
    lambda x: x,  # chroma complexity
    lambda x: x - 240,  # block cost
    lambda x: x,  # fullness
    lambda x: x,  # Y Qp
    lambda x: x,  # U Qp
    lambda x: x,  # V Qp
]

blk_h, blk_w = 8, 16
image_h, image_w = 1080, 1920


def draw_hot(arr, file_name, param, out_path=None):
    graph = np.zeros((image_h, image_w))
    for y in range(blk_h):
        for x in range(blk_w):
            graph[y::blk_h, x::blk_w] = arr
    plt.figure(figsize=(10, 10))
    if param == "mb_complex" or param == "luma_complex" or param == "chroma_complex":
        plt.imshow(graph, cmap="seismic", vmin = 0, vmax = 4)
    elif param == "fullness":
        plt.imshow(graph, cmap="seismic", vmin = 0, vmax = 128)
    else :
        plt.imshow(graph, cmap="seismic")
    plt.title(f"{param}--{file_name}")
    plt.colorbar(shrink=0.66)

    if out_path:
        plt.savefig(f"{out_path}/{file_name}--{param}.png")
    else:
        plt.show()
    plt.close()


def process(file_name, file_path, params_selected, target_image, out_path):
    global image_h, image_w
    fin = open(f"{file_path}/{file_name}_param.txt", 'r')
    case_info = file_name.split("_")
    image_w, image_h = [int(i) for i in case_info[1].split("x")]
    blk_cnt_x, blk_cnt_y = image_w // blk_w, image_h // blk_h

    lines_per_image = blk_cnt_x * blk_cnt_y

    lines = fin.readlines()
    total_lines = len(lines)
    num_images = total_lines // lines_per_image

    if target_image < 0 or target_image >= num_images:
        raise ValueError(f"目标图像号 {target_image} 超出范围，有效范围是 0 到 {num_images - 1}")

    image_data = {}
    for i in params_selected:
        image_data[i] = np.zeros((blk_cnt_y, blk_cnt_x), dtype=np.int16 if params[i][1] == "i" else np.float32)
    for line in lines[target_image * lines_per_image:(target_image + 1) * lines_per_image]:
        trace = line.split()
        y, x = int(trace[1]), int(trace[2])
        for i in params_selected:
            if params[i][1] == "i":
                #print(transformers[i](int(trace[i])))
                image_data[i][y][x] = transformers[i](int(trace[i]))
            else:
                image_data[i][y][x] = transformers[i](float(trace[i]))

    for i in params_selected:
        draw_hot(image_data[i], f"{file_name}_image{target_image}", params[i][0], out_path)


def contrast(file_name, path1, path2, params_selected, target_image, out_path):
    global image_h, image_w
    fin1 = open(f"{path1}/{file_name}_param.txt", 'r')
    fin2 = open(f"{path2}/{file_name}_param.txt", 'r')
    case_info = file_name.split("_")
    image_w, image_h = [int(i) for i in case_info[1].split("x")]
    blk_cnt_x, blk_cnt_y = image_w // blk_w, image_h // blk_h
    lines_per_image = blk_cnt_x * blk_cnt_y

    lines1 = fin1.readlines()
    lines2 = fin2.readlines()
    total_lines = len(lines1)
    num_images = total_lines // lines_per_image

    if target_image < 0 or target_image >= num_images:
        raise ValueError(f"目标图像号 {target_image} 超出范围，有效范围是 0 到 {num_images - 1}")

    image_data1 = {}
    image_data2 = {}
    for i in params_selected:
        image_data1[i] = np.zeros((blk_cnt_y, blk_cnt_x), dtype=np.int16 if params[i][1] == "i" else np.float32)
        image_data2[i] = np.zeros((blk_cnt_y, blk_cnt_x), dtype=np.int16 if params[i][1] == "i" else np.float32)

    for line1, line2 in zip(lines1[target_image * lines_per_image:(target_image + 1) * lines_per_image],
                            lines2[target_image * lines_per_image:(target_image + 1) * lines_per_image]):
        trace = line1.split()
        trace2 = line2.split()
        y, x = int(trace[1]), int(trace[2])
        for i in params_selected:
            if params[i][1] == "i":
                image_data1[i][y][x] = transformers[i](int(trace[i]))
                image_data2[i][y][x] = transformers[i](int(trace2[i]))
            else:
                image_data1[i][y][x] = transformers[i](float(trace[i]))
                image_data2[i][y][x] = transformers[i](float(trace2[i]))

    for i in params_selected:
        image_data1[i][-1][-1] = image_data2[i][-1][-1] = max(np.max(image_data1[i]), np.max(image_data2[i]))
        draw_hot(image_data1[i], f"{file_name}_image{target_image}_1", params[i][0], out_path)
        draw_hot(image_data2[i], f"{file_name}_image{target_image}_2", params[i][0], out_path)


class MainPage:
    def __init__(self):
        root = tk.Tk()
        root.title("码流分析")

        self.path1 = ""
        self.path2 = ""

        self.file_list1 = tk.StringVar()
        self.file_list1.set([])
        self.param_list = tk.StringVar()
        self.param_list.set([item[0] for item in params])

        frame1 = tk.Frame(root)
        y_bar1 = tk.Scrollbar(frame1, orient=tk.VERTICAL)
        self.list_box1 = tk.Listbox(frame1,
                                    yscrollcommand=y_bar1.set,
                                    listvariable=self.file_list1,
                                    height=20, width=50,
                                    selectmode=tk.MULTIPLE,
                                    exportselection=False)
        y_bar1['command'] = self.list_box1.yview
        y_bar1.pack(side=tk.LEFT)
        self.list_box1.pack(side=tk.RIGHT)
        frame1.grid(row=0, column=0)

        self.list_box2 = tk.Listbox(root,
                                    listvariable=self.param_list,
                                    height=20, width=20,
                                    selectmode=tk.MULTIPLE,
                                    exportselection=False)
        self.list_box2.grid(row=0, column=1)

        frame2 = tk.Frame(root)
        tk.Label(frame2, text="输出目录：").grid(row=0, column=0)
        self.out_path = tk.Entry(frame2)
        self.out_path.grid(row=0, column=1)
        tk.Button(frame2, text="浏览", command=self.browse).grid(row=0, column=2)
        self.save = tk.IntVar(value=0)
        tk.Checkbutton(frame2, text="保存图像", variable=self.save).grid(row=0, column=3)
        tk.Label(frame2, text="目标图像号：").grid(row=0, column=4)
        self.target_image = tk.Entry(frame2, width=5)  # 调整宽度为5
        self.target_image.insert(0, "0")  # 设置默认值为0
        self.target_image.grid(row=0, column=5)
        frame2.grid(row=2, column=0, columnspan=2)

        button1 = tk.Button(root, text="选择参数文件目录", command=self.select_path)
        button1.grid(row=1, column=0)

        frame4 = tk.Frame(root)
        button2 = tk.Button(frame4, text="确认", command=self.confirm)
        button2.grid(row=0, column=0)
        button3 = tk.Button(frame4, text="对比", command=self.contrast)
        button3.grid(row=0, column=1)
        frame4.grid(row=4, column=0, columnspan=2)

        # 设置输出目录的默认值为当前脚本所在目录
        self.out_path.insert(0, os.path.dirname(__file__))

        root.mainloop()

    def select_path(self):
        t = askdirectory(initialdir=os.path.dirname(__file__))  # 修改默认路径为脚本路径
        if not t.strip():
            return
        self.path1 = t
        files = []
        for file in os.listdir(self.path1):
            if len(file) >= 10 and file[-10:] == "_param.txt":
                files.append(file[:-10])
        self.file_list1.set(files)

    def browse(self):
        t = askdirectory(initialdir=os.path.dirname(__file__))
        if not t.strip():
            return
        self.out_path.delete(0, "end")
        self.out_path.insert(0, t)

    def confirm(self):
        seqs_selected = self.list_box1.curselection()
        params_selected = self.list_box2.curselection()
        target_image = int(self.target_image.get())
        for i in seqs_selected:
            process(self.list_box1.get(i), self.path1, params_selected, target_image, self.out_path.get() if self.save.get() else None)

    def contrast(self):
        t = askdirectory(initialdir=".")
        if not t.strip():
            return
        self.path2 = t
        seqs_selected = self.list_box1.curselection()
        params_selected = self.list_box2.curselection()
        target_image = int(self.target_image.get())
        for i in seqs_selected:
            contrast(self.list_box1.get(i), self.path1, self.path2, params_selected, target_image, self.out_path.get() if self.save.get() else None)


if __name__ == "__main__":
    MainPage()
