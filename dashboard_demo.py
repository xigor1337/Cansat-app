import tkinter as tk
from tkinter import Label, Frame
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import random as r
import time
import os

if not os.path.isfile("dane.txt"):
    with open("dane.txt", "w", encoding="UTF-8") as f:
        pass

steps = -20

def update_dashboard():
    with open("dane.txt", "a", encoding="UTF-8") as plik:
        global x_data, temperature_data, pressure_data, pm1_data, pm25_data, pm10_data, \
               voc_data, co2_data, lat, lon, altitude, velocity

        # Symulacja ruchu CanSata
        dt = 0.5
        altitude -= velocity * dt
        velocity += 9.81 * dt * 0.4
        if altitude <= 0:
            altitude = 0
            velocity = 0
        
        # Aktualizacja pozycji GPS
        lat += r.uniform(-0.0001, 0.0001) * (1000 - altitude)/1000
        lon += r.uniform(-0.0001, 0.0001) * (1000 - altitude)/1000

        # Generowanie danych czujników
        x_data.append(len(x_data) + 1)
        temperature_data.append(r.randint(20, 40))
        pressure_data.append(r.randint(25, 126))
        pm1_data.append(r.randint(0, 50))
        pm25_data.append(r.randint(0, 50))
        pm10_data.append(r.randint(0, 50))
        voc_data.append(r.randint(0, 500))
        co2_data.append(r.randint(300, 800))

        # Zapisywanie do archiwum
        plik.write(f"{temperature_data[-1]} {pressure_data[-1]} {pm1_data[-1]} {pm25_data[-1]} "
                   f"{pm10_data[-1]} {voc_data[-1]} {co2_data[-1]}\n")

        # Ograniczanie danych
        if len(x_data) > abs(steps):
            for data in [x_data, temperature_data, pressure_data, pm1_data,
                         pm25_data, pm10_data, voc_data, co2_data]:
                data[:] = data[steps:]

        # Aktualizacja wykresów
        for ax, data, title in zip(
            axs,
            [temperature_data, pressure_data, pm1_data, pm25_data, pm10_data, voc_data, co2_data],
            ["Temperatura (°C)", "Ciśnienie (hPa)", "PM1.0", "PM2.5", "PM10", "VOC (ppb)", "CO2 (ppm)"]
        ):
            ax.clear()
            ax.plot(range(len(data)), data, color="lime")
            ax.set_title(title, color="white")
            ax.set_facecolor("black")
            ax.tick_params(axis="x", colors="white")
            ax.tick_params(axis="y", colors="white")
            ax.set_xlim(left=0, right=abs(steps))

        canvas.draw()

        # Aktualizacja panelu współrzędnych
        lat_var.set(f"{lat:.6f}°N")
        lon_var.set(f"{lon:.6f}°E")
        alt_var.set(f"{altitude:.1f} m")
        vel_var.set(f"{velocity:.1f} m/s")
        time_var.set(f"{time.time()-start_time:.1f} s")

    root.after(500, update_dashboard)

# Inicjalizacja danych
x_data = []
temperature_data, pressure_data = [], []
pm1_data, pm25_data, pm10_data = [], [], []
voc_data, co2_data = [], []
lat, lon = 51.1079, 17.0385
altitude = 1000.0
velocity = 0.0

# GUI
root = tk.Tk()
root.title("CanSat Dashboard")
root.geometry("1400x900")
root.configure(bg="black")

# Główne wykresy
fig, axs = plt.subplots(3, 3, figsize=(12, 10))
axs = axs.flatten()
fig.patch.set_facecolor("black")
plt.subplots_adjust(wspace=0.3, hspace=0.5)

for ax in axs[7:]:
    ax.axis("off")

canvas = FigureCanvasTkAgg(fig, master=root)
canvas_widget = canvas.get_tk_widget()
canvas_widget.pack(side="top", fill="both", expand=True)

# Panel współrzędnych w prawym dolnym rogu
coord_frame = Frame(root, bg="darkgreen", bd=3, relief="ridge")
coord_frame.place(relx=1.0, rely=1.0, anchor="se", x=-10, y=-10)

# Nagłówek
Label(coord_frame, text="NAWIGACJA", font=("Consolas", 12, "bold"), 
      fg="white", bg="darkgreen").grid(row=0, column=0, columnspan=2, pady=5)

# Etykiety i wartości
labels = [
    ("Szerokość:", lat_var := tk.StringVar()),
    ("Długość:", lon_var := tk.StringVar()),
    ("Wysokość:", alt_var := tk.StringVar()),
    ("Prędkość:", vel_var := tk.StringVar()),
    ("Czas lotu:", time_var := tk.StringVar())
]

for row, (text, var) in enumerate(labels, start=1):
    Label(coord_frame, text=text, font=("Consolas", 10), 
          fg="white", bg="darkgreen").grid(row=row, column=0, sticky="e", padx=5)
    Label(coord_frame, textvariable=var, font=("Consolas", 10, "bold"), 
          fg="yellow", bg="darkgreen").grid(row=row, column=1, sticky="w", padx=5)

# Inicjalizacja wartości
lat_var.set("51.107900°N")
lon_var.set("17.038500°E")
alt_var.set("1000.0 m")
vel_var.set("0.0 m/s")
time_var.set("0.0 s")

start_time = time.time()
root.after(500, update_dashboard)
root.mainloop()
