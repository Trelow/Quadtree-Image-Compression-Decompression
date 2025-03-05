#include "quadtree.h"

// ...  Queue functions ... //

// Function to create a queue
queue_t *create_queue() {
  queue_t *queue = malloc(sizeof(queue_t));
  queue->front = queue->rear = NULL;
  queue->size = 0;
  return queue;
}

// Function to enqueue a node in a queue
void enqueue(queue_t *queue, tree_t *node) {
  queue_node_t *temp = malloc(sizeof(queue_node_t));
  temp->node = node;
  temp->next = NULL;
  if (queue->rear == NULL) {
    queue->front = queue->rear = temp;
    queue->size++;
    return;
  }
  queue->rear->next = temp;
  queue->rear = temp;
  queue->size++;
}

// Function to dequeue a node from a queue
tree_t *dequeue(queue_t *queue) {
  if (queue->front == NULL) {
    return NULL;
  }
  queue_node_t *temp = queue->front;
  tree_t *node = temp->node;
  queue->front = queue->front->next;
  if (queue->front == NULL) {
    queue->rear = NULL;
  }
  free(temp);
  queue->size--;
  return node;
}

// Function to check if a queue is empty
int is_queue_empty(queue_t *queue) {
  if (queue->size == 0) {
    return 1;
  } else
    return 0;
}

// Function to free a queue
void free_queue(queue_t *queue) {
  while (!is_queue_empty(queue)) {
    dequeue(queue);
  }
  free(queue);
}

// ... Image functions ... //

// Function to read a PPM image file
pixel_t **read_image(const char *filename, int *width, int *height) {
  int i;
  // Open file for reading
  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    return NULL;
  }
  // Read image format
  char format[3];
  fscanf(file, "%2s\n", format);
  if (format[0] != 'P' || format[1] != '6') {
    printf("Image format must be 'P6')\n");
    return NULL;
  }
  // Read image size information
  fscanf(file, "%d %d\n", width, height);
  // Check if image is 2^n x 2^n
  int n = log2(*width);
  if (*width != *height || *width != pow(2, n)) {
    printf("Invalid image size (must be 2^n x 2^n)\n");
    return NULL;
  }
  // Read color range
  int color_range;
  fscanf(file, "%d", &color_range);
  fseek(file, 1, SEEK_CUR);
  if (color_range != 255) {
    printf("Invalid color range value (must be 255)\n");
    return NULL;
  }
  // Allocate memory for image data
  pixel_t **image = malloc(*height * sizeof(pixel_t *));
  for (i = 0; i < *height; i++) {
    image[i] = malloc(*width * sizeof(pixel_t));
  }
  int x, y;
  for (y = 0; y < *height; y++) {
    for (x = 0; x < *width; x++) {
      fread(&image[y][x], sizeof(pixel_t), 1, file);
    }
  }
  fclose(file);
  return image;
}

// Function to recreate image from quaternary tree
void recreate_image(tree_t *tree, unsigned int size, int pivot[2],
                    pixel_t **image) {
  int i, j;
  // If node is not a leaf, recursively recreate its children
  if (!tree->is_leaf) {
    int new_pivot[2];
    new_pivot[0] = pivot[0];
    new_pivot[1] = pivot[1];
    recreate_image(tree->child[0], size / 2, new_pivot, image);
    new_pivot[0] = pivot[0] + size / 2;
    new_pivot[1] = pivot[1];
    recreate_image(tree->child[1], size / 2, new_pivot, image);
    new_pivot[0] = pivot[0] + size / 2;
    new_pivot[1] = pivot[1] + size / 2;
    recreate_image(tree->child[2], size / 2, new_pivot, image);
    new_pivot[0] = pivot[0];
    new_pivot[1] = pivot[1] + size / 2;
    recreate_image(tree->child[3], size / 2, new_pivot, image);
  } // If node is a leaf, fill the square with the pixel value
  else if (tree->is_leaf) {
    for (i = 0; i < (int)size; i++) {
      for (j = 0; j < (int)size; j++) {
        image[pivot[1] + i][pivot[0] + j] = tree->pixel;
      }
    }
  }
}

// Function to write image to PPM file
void write_image_to_PPM(pixel_t **image, unsigned int size, char *file_name) {
  int i, j;
  FILE *file = fopen(file_name, "wb");
  fprintf(file, "P6\n%d %d\n255\n", size, size);
  for (i = 0; i < (int)size; i++) {
    for (j = 0; j < (int)size; j++) {
      fwrite(&(image[i][j]), sizeof(pixel_t), 1, file);
    }
  }
  fclose(file);
}

// Function to free image
void free_image(pixel_t **image, int size) {
  int i;
  for (i = 0; i < size; i++) {
    free(image[i]);
  }
  free(image);
}

// ... Quadtree functions ... //

// Function to initialize a quadtree node
tree_t *init_tree() {
  int i;
  tree_t *tree = malloc(sizeof(tree_t));
  tree->is_leaf = 0;
  tree->pixel.r = 0;
  tree->pixel.g = 0;
  tree->pixel.b = 0;
  for (i = 0; i < 4; i++) {
    tree->child[i] = NULL;
  }
  return tree;
}

// Function to get height of a quadtree
int get_height(tree_t *tree) {
  int i;
  if (tree == NULL) {
    return 0;
  }
  int max = 0;
  for (i = 0; i < 4; i++) {
    int height = get_height(tree->child[i]);
    if (height > max) {
      max = height;
    }
  }
  return max + 1;
}

// Function to calculate mean color of a block
void calculate_mean(pixel_t **image, int size, int pivot[2],
                    unsigned long long *r, unsigned long long *g,
                    unsigned long long *b, unsigned long long *mean) {
  int i, j;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      *r += image[pivot[1] + i][pivot[0] + j].r;
      *g += image[pivot[1] + i][pivot[0] + j].g;
      *b += image[pivot[1] + i][pivot[0] + j].b;
    }
  }
  *r = *r / (size * size);
  *g = *g / (size * size);
  *b = *b / (size * size);
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      *mean += (*r - image[pivot[1] + i][pivot[0] + j].r) *
                   (*r - image[pivot[1] + i][pivot[0] + j].r) +
               (*g - image[pivot[1] + i][pivot[0] + j].g) *
                   (*g - image[pivot[1] + i][pivot[0] + j].g) +
               (*b - image[pivot[1] + i][pivot[0] + j].b) *
                   (*b - image[pivot[1] + i][pivot[0] + j].b);
    }
  }
  *mean = *mean / (3 * size * size);
}

// Function to create recursively a quadtree from an image
tree_t *create_compression_quadtree(pixel_t **image, int size, int pivot[2],
                                    const float threshold, int *similar_blocks,
                                    int *largest_block_dimension) {
  int i;
  tree_t *tree = init_tree();

  // If block size is 1 pixel, then the node is a leaf
  if (size == 1) {
    tree->pixel = image[pivot[1]][pivot[0]];
    tree->is_leaf = 1;
    (*similar_blocks)++;
  } // If block size is greater than 1 pixel, calculate mean value, if it is
    // greater then threshold, recursively divide block in 4 smaller ones, else
    // attribute average rgb color to block
  else if (size > 1) {
    unsigned long long r = 0, g = 0, b = 0, mean = 0;
    //  Calculate mean
    calculate_mean(image, size, pivot, &r, &g, &b, &mean);
    //  Check if mean is less than threshold
    if (mean <= threshold) {
      tree->pixel.r = r;
      tree->pixel.g = g;
      tree->pixel.b = b;
      tree->is_leaf = 1;
      for (i = 0; i < 4; i++) {
        tree->child[i] = NULL;
      }
      if (size > *largest_block_dimension) {
        *largest_block_dimension = size;
      }
      (*similar_blocks)++;
    } else {
      // Division of block in 4 smaller ones
      int new_pivot[2];
      new_pivot[0] = pivot[0];
      new_pivot[1] = pivot[1];
      tree->child[0] =
          create_compression_quadtree(image, size / 2, new_pivot, threshold,
                                      similar_blocks, largest_block_dimension);
      new_pivot[0] = pivot[0] + size / 2;
      new_pivot[1] = pivot[1];
      tree->child[1] =
          create_compression_quadtree(image, size / 2, new_pivot, threshold,
                                      similar_blocks, largest_block_dimension);
      new_pivot[0] = pivot[0] + size / 2;
      new_pivot[1] = pivot[1] + size / 2;
      tree->child[2] =
          create_compression_quadtree(image, size / 2, new_pivot, threshold,
                                      similar_blocks, largest_block_dimension);
      new_pivot[0] = pivot[0];
      new_pivot[1] = pivot[1] + size / 2;
      tree->child[3] =
          create_compression_quadtree(image, size / 2, new_pivot, threshold,
                                      similar_blocks, largest_block_dimension);
    }
  }
  return tree;
}

// Function to write compressed quadtree in binary file
void write_compression_quadtree(tree_t *tree, unsigned int size,
                                char *file_name) {
  int i;
  FILE *binary_file = fopen(file_name, "wb");
  // Write size of image
  fwrite(&size, sizeof(unsigned int), 1, binary_file);
  // Create queue and enqueue root
  queue_t *queue = create_queue();
  enqueue(queue, tree);
  // Traverse tree in level order and write nodes in binary file
  while (!is_queue_empty(queue)) {
    tree_t *node = dequeue(queue);
    fwrite(&(node->is_leaf), sizeof(unsigned char), 1, binary_file);
    if (node->is_leaf) {
      fwrite(&node->pixel, sizeof(pixel_t), 1, binary_file);
    } else {
      for (i = 0; i < 4; i++) {
        if (node->child[i] != NULL)
          enqueue(queue, node->child[i]);
      }
    }
  }
  free_queue(queue);
  fclose(binary_file);
}

// Function to recreate quadtree from compressed binary file
tree_t *recreate_quadtree(char *filename, unsigned int *size) {
  int i;
  FILE *binaryFile = fopen(filename, "rb");
  if (binaryFile == NULL)
    return NULL;

  // Create root node
  tree_t *tree = malloc(sizeof(tree_t));
  fread(size, sizeof(unsigned int), 1, binaryFile);
  fread(&(tree->is_leaf), sizeof(unsigned char), 1, binaryFile);
  // If root is leaf, read pixel value
  if (tree->is_leaf) {
    tree->child[0] = NULL;
    tree->child[1] = NULL;
    tree->child[2] = NULL;
    tree->child[3] = NULL;
    fread(&tree->pixel, sizeof(pixel_t), 1, binaryFile);
  } // If root is not leaf, read children
  else {
    // Create queue and enqueue root
    queue_t *queue = create_queue();
    enqueue(queue, tree);
    // Traverse tree in level order and read nodes from binary file
    while (!is_queue_empty(queue)) {
      tree_t *node = dequeue(queue);
      for (i = 0; i < 4; i++) {
        node->child[i] = malloc(sizeof(tree_t));
        fread(&(node->child[i]->is_leaf), sizeof(unsigned char), 1, binaryFile);
        if (node->child[i]->is_leaf) {
          node->child[i]->child[0] = NULL;
          node->child[i]->child[1] = NULL;
          node->child[i]->child[2] = NULL;
          node->child[i]->child[3] = NULL;
          fread(&node->child[i]->pixel, sizeof(pixel_t), 1, binaryFile);
        } else {
          enqueue(queue, node->child[i]);
        }
      }
    }
    free_queue(queue);
  }
  fclose(binaryFile);
  return tree;
}

// Function to free quaternary tree
void free_tree(tree_t *tree) {
  int i;
  if (tree == NULL) {
    return;
  }
  for (i = 0; i < 4; i++) {
    free_tree(tree->child[i]);
  }
  free(tree);
}

// ... Command manager functions ... //

// Function to check if a string is number
int is_number(char *string) {
  int i;
  for (i = 0; i < (int)strlen(string); i++) {
    if (!isdigit(string[i])) {
      return 0;
    }
  }
  return 1;
}

int command_manager(int argc, char *argv[]) {
  int i;
  // If arguments are less than 4, print error
  if (argc < 4) {
    printf("Error: wrong number of arguments\n");
    return 1;
  }
  if (strcmp(argv[1], "-c1") == 0 || strcmp(argv[1], "-c2") == 0) {
    // If arguments are more than 5, print error
    if (argc != 5) {
      printf("Error: wrong number of arguments\n");
      return 1;
    }
    // If threshold is not a number, print error
    if (is_number(argv[2]) == 0) {
      printf("Error: threshold is not an integer number\n");
      return 1;
    }
    int similar_blocks = 0;
    int largest_block_dimension = 1;
    int pivot[2] = {0, 0};
    int threshold = atof(argv[2]);
    int size;

    // Read image
    pixel_t **image = read_image(argv[3], &size, &size);
    // If image is NULL, print error
    if (image == NULL) {
      printf("Error reading image\n");
      return 1;
    }
    // Create quadtree
    tree_t *tree =
        create_compression_quadtree(image, size, pivot, threshold,
                                    &similar_blocks, &largest_block_dimension);
    free_image(image, size);

    // Task 1
    if (strcmp(argv[1], "-c1") == 0) {
      FILE *output = fopen(argv[4], "w");
      // If output file is NULL, print error
      if (output == NULL) {
        printf("Error opening output file\n");
        return 1;
      }
      fprintf(output, "%d\n", get_height(tree));
      fprintf(output, "%d\n", similar_blocks);
      fprintf(output, "%d\n", largest_block_dimension);
      fclose(output);
    } // Task 2
    else if (strcmp(argv[1], "-c2") == 0) {
      write_compression_quadtree(tree, size, argv[4]);
    }
    free_tree(tree);
  } // Task 3
  else if (strcmp(argv[1], "-d") == 0) {
    // If arguments are more than 4, print error
    if (argc != 4) {
      printf("Error: wrong number of arguments\n");
      return 1;
    }
    unsigned int size;
    // Recreate quadtree
    tree_t *tree = recreate_quadtree(argv[2], &size);
    // If tree is NULL, print error
    if (tree == NULL) {
      printf("Error recreating quadtree\n");
      return 1;
    }
    // Allocate memory for image
    pixel_t **image = malloc(size * sizeof(pixel_t *));
    for (i = 0; i < (int)size; i++) {
      image[i] = malloc(size * sizeof(pixel_t));
    }

    // Recreate image and write it to PPM file
    int pivot[2] = {0, 0};
    recreate_image(tree, size, pivot, image);
    write_image_to_PPM(image, size, argv[3]);

    free_image(image, size);
    free_tree(tree);
  } // If arguments are not valid, print error
  else {
    printf("Invalid arguments\n");
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int has_error = command_manager(argc, argv);
  // If program has error, return 1
  if (has_error)
    return 1;
  return 0;
}