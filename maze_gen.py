import random

def generate_final_labyrinth_with_fixed_bottom(width, height):
    # Ensure dimensions are odd for proper maze structure
    if width % 2 == 0: width -= 1
    if height % 2 == 0: height -= 1

    # Adjust dimensions to account for the surrounding wall
    width += 1
    height += 1

    # Create a grid filled with walls (1)
    maze = [[1 for _ in range(width)] for _ in range(height)]

    # Enhanced DFS for generating long corridors and diverse paths
    def dfs(x, y, depth=0):
        directions = [(0, 2), (2, 0), (0, -2), (-2, 0)]
        random.shuffle(directions)

        for dx, dy in directions:
            nx, ny = x + dx, y + dy

            # Check if the new cell is within bounds and still a wall
            if 1 <= nx < height - 1 and 1 <= ny < width - 1 and maze[nx][ny] == 1:
                # Remove walls between cells
                maze[x + dx // 2][y + dy // 2] = 0
                maze[nx][ny] = 0
                dfs(nx, ny, depth + 1)

    # Start DFS from a random cell
    start_x, start_y = random.randrange(1, height, 2), random.randrange(1, width, 2)
    maze[start_x][start_y] = 0
    dfs(start_x, start_y)

    # Ensure the maze is surrounded by walls (frame)
    for i in range(width):
        maze[0][i] = maze[height - 1][i] = 1
    for i in range(height):
        maze[i][0] = maze[i][width - 1] = 1

    # Adjust the bottom rows to avoid double walls
    maze[-2] = [1] + [0 if i % 2 == 0 else 1 for i in range(width - 2)] + [1]
    maze[-1] = [1] * width

    # Validate and adjust connections to ensure accessibility
    for i in range(1, height - 1):
        for j in range(1, width - 1):
            # Remove isolated corridors or fix inaccessible areas
            if maze[i][j] == 0:
                neighbors = sum(1 for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]
                                if maze[i + dx][j + dy] == 0)
                if neighbors == 0:
                    maze[i][j] = 1

    # Add treasures (3 and 4) in logical positions (dead ends)
    for i in range(1, height - 3, 2):  # Avoid the last three rows
        for j in range(1, width - 1, 2):
            if maze[i][j] == 0:
                # Check if this cell is a dead end
                neighbors = sum(1 for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]
                                if maze[i + dx][j + dy] == 0)
                if neighbors == 1 and random.random() > 0.6:  # Add treasure to some dead ends
                    maze[i][j] = random.choice([3, 4])

    return maze

def save_maze_to_file(filename, maze):
    """
    Save the generated maze to a file in the required format.
    """
    with open(filename, "w") as file:
        for row in maze:
            formatted_row = "{" + ",".join(map(str, row)) + "},"
            file.write(formatted_row + "\n")

# Generate the maze
final_labyrinth_fixed_bottom = generate_final_labyrinth_with_fixed_bottom(22, 24)

# Save the maze to a file
save_maze_to_file("mazeGen.txt", final_labyrinth_fixed_bottom)

print("Labyrinth has been successfully saved to mazeGen.txt.")
