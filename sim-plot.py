import matplotlib.pyplot as plt
import os
import numpy as np
import sys

# niebieski to kanal voltage in
# pomaranczowy to velocity in



class Plotter():
    def __init__(self) -> None:
        self.total_samples: int = 0
        self.time = []
        self.velocity = []
        self.vmfc = []
        self.pos = []
        self.neg = []
        self.vref = []
        self.vin = []

    def get_data(self, file):
        if not os.path.isfile(file):
            print("Given arg is not a file!")
        with open(file, "r") as f:
            for line in f:
                # print(line)
                word = line.split()
                if word[0] != "XXX":
                    continue
                self.time.append(float(word[1]))
                self.velocity.append(float(word[2]))
                self.vmfc.append(float(word[3]))
                self.pos.append(float(word[9]))
                self.neg.append(float(word[10]))
                self.vref.append(float(word[11])*3-5)
                self.vin.append(float(word[8]))
                
        self.l: int = len(self.time)

    def plot(self):
        if not self.time:
            print("No data to plot!")
            return

        # self.samples_ch1 = np.subtract(self.samples_ch1, np.mean(self.samples_ch1))
        # self.samples_ch2 = np.subtract(self.samples_ch2, np.mean(self.samples_ch2))
        # l = len(self.samples_ch1)
        # AxisX = np.linspace(0,((l-1) / self.SAMPLING_FREQ_HZ), l)
        figure, axis = plt.subplots(4, 1)
        axis[0].plot(self.time, self.velocity)
        axis[0].set_title("Velocity")
        axis[1].plot(self.time, self.vmfc)
        axis[1].set_title("VMFC")
        axis[1].plot(self.time, self.vref)
        axis[2].set_title("vref")
        axis[3].plot(self.time, self.vin)
        axis[3].set_title("vin")
        plt.grid(True)
        plt.show()
        figure1, axis1 = plt.subplots(1, 1)
        axis1.plot(self.time, self.vmfc, label="Vmfc")
        axis1.plot(self.time, self.vref, label="Vref")
        axis1.set_title("VMFC")
        axis1.set_xlabel("Czas [s]")
        axis1.set_ylabel("Napięcie [V]")
        axis1.legend(loc="upper right")
        plt.grid(True)
        plt.show()

    # def plot_fft(self):
    #     if not self.total_samples:
    #         print("No data to plot!")
    #         return

    #     self.samples_ch1 = np.subtract(self.samples_ch1, np.mean(self.samples_ch1))
    #     self.samples_ch2 = np.subtract(self.samples_ch2, np.mean(self.samples_ch2))
    #     # self.test_signal()
    #     l = len(self.samples_ch1)

    #     f_plot = np.fft.fftfreq(l, 1/self.SAMPLING_FREQ_HZ)
    #     data_fft = 20*np.log10(np.abs(np.fft.fft(self.samples_ch1))*2/l)
    #     if self.double_channels:
    #         data2_fft = 20*np.log10(np.abs(np.fft.fft(self.samples_ch2))*2/l)
    #         figure, axis = plt.subplots(2, 1)
    #         axis[0].plot(f_plot[1:l//2], data_fft[1:l//2])
    #         axis[0].set_title("Channel 1")
    #         axis[1].plot(f_plot[1:l//2], data2_fft[1:l//2])
    #         axis[1].set_title("Channel 2")
    #     else:
    #         plt.plot(f_plot[1:l//2], data_fft[1:l//2])
    #         plt.title('FFT of the Signal')
    #         plt.xlabel('Frequency (Hz)')
    #         plt.ylabel('Magnitude [dB]')
    #     plt.grid(True)
    #     plt.show()   

def find(name, path):
    for root, dirs, files in os.walk(path):
        if name in files:
            return os.path.join(root, name)

def main():
    if len(sys.argv) < 2:
        return
    
    filename = sys.argv[1]
    bin_file = find(filename, os. getcwd())
    # bin_file = "./sim3.txt"    
    plotter = Plotter()
    plotter.get_data(bin_file)
    plotter.plot()

if __name__ == "__main__":
    main()