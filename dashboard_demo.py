import tkinter as tk
from tkinter import Label, Frame
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import random as r
import time
import os

if os.path.isfile("dane.txt") == False:
    plik = open("dane.txt", "x", encoding="UTF-8")
    plik.close()

steps = -20
def update_dashboard():
    plik = open("dane.txt", "a", encoding="UTF-8")
    global x_data, temperature_data, pressure_data, pm1_data, pm25_data, pm10_data, voc_data, co2_data, battery_data, time_since_fall_data

    # wprowadzanie dancyh
    x_data.append(len(x_data) + 1)
    temperature_data.append(r.randint(20, 40))
    pressure_data.append(r.randint(25, 126))
    pm1_data.append(r.randint(0, 50))
    pm25_data.append(r.randint(0, 50))
    pm10_data.append(r.randint(0, 50))
    voc_data.append(r.randint(0, 500))
    co2_data.append(r.randint(300, 800))
    battery_data.append(r.randint(0, 100))
    time_since_fall_data.append(round(time.time() - start_time, 2))

    plik.writelines(
        str(temperature_data[-1]) + " " +
        str(pressure_data[-1]) + " " +
        str(pm1_data[-1]) + " " +
        str(pm25_data[-1]) + " " +
        str(pm10_data[-1]) + " " +
        str(voc_data[-1]) + " " +
        str(co2_data[-1]) + " " +
        str(battery_data[-1]) + " " +
        str(time_since_fall_data[-1]) + "\n")


    # Ensure the chart continues to show data flow instead of disappearing
    if len(x_data) > abs(steps):
        x_data = x_data[steps:]
        temperature_data = temperature_data[steps:]
        pressure_data = pressure_data[steps:]
        pm1_data = pm1_data[steps:]
        pm25_data = pm25_data[steps:]
        pm10_data = pm10_data[steps:]
        voc_data = voc_data[steps:]
        co2_data = co2_data[steps:]
        battery_data = battery_data[steps:]
        time_since_fall_data = time_since_fall_data[steps:]

    for ax, data, title in zip(
        axs,
        [temperature_data, pressure_data, pm1_data, pm25_data, pm10_data, voc_data, co2_data, battery_data, time_since_fall_data],
        ["Temperature (°C)", "Pressure", "PM1.0", "PM2.5", "PM10", "VOC", "CO2", "Battery (%)", "Time Since Fall (s)"],
    ):
        ax.clear()
        ax.plot(range(len(data)), data, color="green")
        ax.set_title(title, color="white")
        ax.set_facecolor("black")
        ax.tick_params(axis="x", colors="white")
        ax.tick_params(axis="y", colors="white")
        ax.set_xlim(left=0, right=20)

    canvas.draw()

    time_label.config(text=f"Time: {round(time.time() - start_time, 2)} min")
    battery_label.config(text=f"Battery: {r.randint(50, 100):.1f}%")
    free_fall_label.config(text="Free Fall: No" if r.randint(0, 1) > 0.5 else "Free Fall: Yes")

    root.after(500, update_dashboard)

x_data = []
temperature_data, pressure_data, pm1_data, pm25_data, pm10_data, voc_data, co2_data, battery_data, time_since_fall_data = [], [], [], [], [], [], [], [], []

root = tk.Tk()
root.title("Air Quality Real-time Dashboard")
root.geometry("1200x800")
root.configure(bg="black")

fig, axs = plt.subplots(3, 3, figsize=(10, 9))
axs = axs.flatten()

fig.patch.set_facecolor("black")
plt.subplots_adjust(wspace=0.3, hspace=0.3)

canvas = FigureCanvasTkAgg(fig, master=root)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side="top", fill="both", expand=True)

frame = Frame(root, bg="black")
frame.pack(fill="x", pady=10)

time_label = Label(frame, text="Time: 0.00 min", font=("Helvetica", 16), fg="white", bg="black")
time_label.pack(side="left", padx=20)

battery_label = Label(frame, text="Battery: 100%", font=("Helvetica", 16), fg="white", bg="black")
battery_label.pack(side="left", padx=20)

free_fall_label = Label(frame, text="Free Fall: No", font=("Helvetica", 16), fg="white", bg="black")
free_fall_label.pack(side="left", padx=20)

start_time = time.time()

root.after(500, update_dashboard)
root.mainloop()
