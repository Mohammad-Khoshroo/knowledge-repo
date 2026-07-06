# Full Reference-Style Guide to `matplotlib` and `pyplot`

## 1. Introduction

`matplotlib` is the most widely used plotting library in Python. `pyplot` is a submodule of `matplotlib` designed to provide a MATLAB-like plotting framework.

```python
import matplotlib.pyplot as plt
```

---

## 2. Basic Plotting

### 2.1 Line Plot

```python
plt.plot(x, y, label='Line')
```

- `x`, `y`: lists or arrays of data
- `label`: for use in legend

### 2.2 Display Plot

```python
plt.show()
```

### 2.3 Save Plot

```python
plt.savefig('filename.png', dpi=300, bbox_inches='tight')
```

---

## 3. Plot Customization

### 3.1 Title and Labels

```python
plt.title("Title")
plt.xlabel("X Axis")
plt.ylabel("Y Axis")
```

### 3.2 Grid and Legend

```python
plt.grid(True)
plt.legend()
```

### 3.3 Axis Limits

```python
plt.xlim([xmin, xmax])
plt.ylim([ymin, ymax])
```

---

## 4. Styling Lines and Markers

```python
plt.plot(x, y, color='red', linestyle='--', linewidth=2, marker='o', markersize=8)
```

- `color`: 'r', '#FF0000', etc.
- `linestyle`: '-', '--', '-.', ':'
- `marker`: 'o', 's', '^', '\*', '.', etc.
- `linewidth`, `markersize`: control thickness/size

---

## 5. Subplots and Layouts

### 5.1 Subplot

```python
plt.subplot(nrows, ncols, index)
```

### 5.2 `plt.subplots()` (preferred for complex layout)

```python
fig, ax = plt.subplots(2, 2)
ax[0, 0].plot(x, y)
```

### 5.3 Adjust spacing

```python
plt.tight_layout()
```

---

## 6. Common Plot Types

### 6.1 Scatter Plot

```python
plt.scatter(x, y, c='blue', s=40)
```

### 6.2 Bar Chart

```python
plt.bar(categories, values)
```

### 6.3 Histogram

```python
plt.hist(data, bins=20, color='green')
```

### 6.4 Pie Chart

```python
plt.pie(sizes, labels=labels, autopct='%1.1f%%')
```

### 6.5 Box Plot

```python
plt.boxplot(data)
```

### 6.6 Heatmap (using `imshow`)

```python
plt.imshow(matrix, cmap='hot', interpolation='nearest')
```

---

## 7. Working with Figures and Axes

### 7.1 Create figure and axes manually

```python
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
ax.plot(x, y)
```

### 7.2 Multiple axes

```python
fig, axs = plt.subplots(2, 1)
axs[0].plot(x1, y1)
axs[1].plot(x2, y2)
```

### 7.3 Shared axes

```python
fig, axs = plt.subplots(2, 2, sharex=True, sharey=True)
```

### 7.4 Figure size

```python
plt.figure(figsize=(10, 6))
```

---

## 8. Customizing Ticks and Spines

### 8.1 Tick positions and labels

```python
plt.xticks([0, 1, 2], ['A', 'B', 'C'])
```

### 8.2 Remove or style spines (borders)

```python
ax.spines['top'].set_visible(False)
ax.spines['right'].set_color('none')
```

---

## 9. Annotations and Text

### 9.1 Add annotation

```python
plt.annotate('Peak', xy=(x, y), xytext=(x+1, y+1),
             arrowprops=dict(facecolor='black'))
```

### 9.2 Add text

```python
plt.text(x, y, 'Note', fontsize=12)
```

---

## 10. Images and Imshow

### 10.1 Display image

```python
img = plt.imread('image.png')
plt.imshow(img)
```

### 10.2 Show matrix

```python
plt.imshow(matrix, cmap='viridis')
plt.colorbar()
```

---

## 11. Animation

```python
from matplotlib.animation import FuncAnimation

def animate(i):
    line.set_ydata(np.sin(x + i / 10))
    return line,

fig, ax = plt.subplots()
line, = ax.plot(x, np.sin(x))
ani = FuncAnimation(fig, animate, frames=100, interval=50)
plt.show()
```

---

## 12. 3D Plots

```python
from mpl_toolkits.mplot3d import Axes3D
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot3D(x, y, z)
```

---

## 13. Tips for Publication-Quality Plots

- Use `figsize=(width, height)` in inches
- Set `dpi=300` for print-quality resolution
- Use `bbox_inches='tight'` in `savefig`
- Customize fonts using:

```python
plt.rcParams['font.size'] = 12
plt.rcParams['font.family'] = 'serif'
```

---

## 14. Export and Save

```python
plt.savefig("plot.png", dpi=300)
plt.savefig("plot.pdf")
```
