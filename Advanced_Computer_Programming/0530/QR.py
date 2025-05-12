import numpy as np
from matplotlib import pyplot as plt

v1 = np.array([2, 3])
v2 = np.array([1, 1])
A = np.column_stack((v1, v2))
Q, R = np.linalg.qr(A)
basis1 = Q[:, 0]
basis2 = Q[:, 1]
if np.linalg.norm(basis2) > 1e-6:
    # Plot both basis vectors
    plt.arrow(0, 0, basis1[0], basis1[1], head_width=0.1, head_length=0.2, color='red', label='1')
    plt.arrow(0, 0, basis2[0], basis2[1], head_width=0.1, head_length=0.2, color='blue', label='2')
else:
    plt.arrow(0, 0, basis1[0], basis1[1], head_width=0.1, head_length=0.2, color='red', label='Basis')
plt.xlabel("X")
plt.ylabel("Y")
plt.title("411286029")
plt.legend()
plt.xlim([-max(np.abs(basis1)) - 0.5, max(np.abs(basis1)) + 0.5])
plt.ylim([-max(np.abs(basis1)) - 0.5, max(np.abs(basis1)) + 0.5])
plt.axhline(0, color='black',linewidth=1)
plt.axvline(0, color='black',linewidth=1)
plt.grid(color = 'gray', linestyle = '--', linewidth = 0.5)
plt.legend()
plt.savefig("qr.png")
plt.show()
