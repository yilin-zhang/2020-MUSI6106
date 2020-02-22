import numpy as np
import matplotlib.pyplot as plt

data_cpp = np.loadtxt("MajorTom16.wav.txt").T
data_matlab = np.loadtxt("MajorTomSpectrogram.txt")

diff = np.sqrt(np.mean(np.power((data_cpp - data_matlab), 2)))
print(diff)

plt.title('CPP spectrogram')
plt.ylabel('Frequency [Hz]')
plt.xlabel('Time [sec]')
plt.pcolormesh(data_cpp)
plt.show()

plt.title('MATLAB spectrogram')
plt.ylabel('Frequency [Hz]')
plt.xlabel('Time [sec]')
plt.pcolormesh(data_matlab)
plt.show()
