import numpy as np
import matplotlib.pyplot as plt

v1 = np.array([2, 3])
v2 = np.array([1, 1])

A = np.column_stack((v1, v2))

Q, R = np.linalg.qr(A)
basis1 = Q[:, 0]
basis2 = Q[:, 1]
print(basis1[0], basis1[1])
print(basis2[0], basis2[1])
plt.figure()
plt.quiver(0, 0, v1[0], v1[1], angles='xy', scale_units='xy', scale=1, color='r', label='v1')
plt.quiver(0, 0, v2[0], v2[1], angles='xy', scale_units='xy', scale=1, color='b', label='v2')
plt.quiver(0, 0, basis1[0], basis1[1], angles='xy', scale_units='xy', scale=1, color='g', label='basis1')
plt.quiver(0, 0, basis2[0], basis2[1], angles='xy', scale_units='xy', scale=1, color='m', label='basis2')
plt.title(f"411286029")
plt.xlim(-1, 5)
plt.ylim(-1, 5)
plt.axhline(0, color='black',linewidth=1)
plt.axvline(0, color='black',linewidth=1)
plt.grid(color = 'gray', linestyle = '--', linewidth = 0.5)
plt.legend()
plt.savefig('qr.png')
plt.show()
