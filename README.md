# Quadtree Image Compression & Decompression

This project demonstrates a **quadtree-based algorithm** to **compress** and **decompress** square images (PPM format) with side lengths that are powers of two. The code is written in **C** and covers three main tasks:

1. **Analyze & Construct** a quadtree from an image, reporting statistics.  
2. **Compress** the image quadtree to a **binary file**.  
3. **Decompress** the quadtree from a binary file back into a PPM image.

---

## Table of Contents
1. [Overview](#overview)  
2. [Data Structures](#data-structures)  
3. [Features](#features)  
4. [Usage](#usage)  
---

## Overview
- **Input**: A square **PPM** image (`P6` format, side = 2^n).  
- **Core Algorithm**: Uses **quadtree decomposition** to decide whether a block of pixels is uniform enough (based on a threshold). 
  - If uniform, represent as a **leaf node** with an average color.  
  - Otherwise, subdivide into **4 child blocks**.  
- **Output**:  
  1. **Task 1** (Analysis): Reports quadtree height, number of leaf blocks, and size of the largest non-divided block.  
  2. **Task 2** (Compression): Stores the quadtree in a **binary** format.  
  3. **Task 3** (Decompression): Reads the binary quadtree and reconstructs the **PPM** image.

---

## Data Structures

1. **`pixel_t`**  
   - Holds the **red, green, blue** values (`unsigned char`) of a single pixel.  
2. **`tree_t`** (Quadtree Node)  
   - **`pixel`**: The average color for leaf nodes.  
   - **`is_leaf`**: Flag indicating if this node is a leaf.  
   - **`child[4]`**: Pointers to child nodes (sub-blocks).  
3. **`queue_t`** (Level-Order Traversal)  
   - Basic **linked-list** queue used for breadth-first (level-order) operations when writing or reading the quadtree.

---

## Features

1. **Quadtree Construction**  
   - Recursively divides the image block into 4 sub-blocks if the variance exceeds a threshold.  
   - Leaf nodes store an **average RGB** for that block.

2. **Statistics** (Task 1)  
   - **Height of quadtree**.  
   - **Number of similar (leaf) blocks**.  
   - **Largest undivided block size** in pixels.

3. **Quadtree to Binary** (Task 2)  
   - Level-order traversal: writes **leaf flags** and **pixel values** (if leaf) to a binary file.

4. **Quadtree from Binary** (Task 3)  
   - Rebuilds the tree in level order using the stored leaf flags and pixel values.  
   - **Decompression**: Recursively fills out the image with the node’s color if leaf, or subdivides further if internal.

---

## Usage

1. **Compile**:  
   - Run `make` to compile the code.  
   - The executable `quadtree` will be created.
## 2. Running Commands

### **Task 1: Create Quadtree & Compute Statistics**
```bash
./quadtree -c1 <threshold> <input.ppm> <output_stats.txt>
```
### **Task 2: Compress Image to Binary**
```bash
./quadtree -c2 <threshold> <input.ppm> <output.bin>
```

### **Task 3: Decompress Binary to Image**
```bash
./quadtree -d <input.bin> <output.ppm>
```

### Normal Image

![alt text](image.png)

### Compressed Image (THRESHOLD = 40)

![alt text](image-1.png)
