import random
import csv

def generate_memory(size, data_bits):
    memory = []
    for _ in range(size):
        if random.randint(0, 9) == 0:  # ~10% chance of all 'x'
            memory.append("x" * data_bits)
        else:
            max_val = (1 << data_bits) - 1
            val = random.randint(0, max_val)
            memory.append(f"{val:0{data_bits}b}")  # format to data_bits width
    return memory

def write_memory_to_csv(filename, memory):
    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["address", "data"])
        for i, data in enumerate(memory):
            if 'x' in data.lower():
                data = 'x' * len(data)  # Ensure full 'x' bits
            writer.writerow([f"0x{i:03X}", data])

if __name__ == "__main__":
    data_bits = 8  # Example: generate 24-bit memory words
    memory_size = 50
    memory = generate_memory(memory_size, data_bits)
    write_memory_to_csv("_memory_input.csv", memory)
    print(f"Memory dump with {data_bits}-bit data written to memory_dump.csv")
