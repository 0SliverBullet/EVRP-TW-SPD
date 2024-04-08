import os 
def read_instance_file(file_path):
    data = {'StringID': [], 'Type': [], 'x': [], 'y': [], 'Delivery': [], 'Pickup': [],
            'ReadyTime': [], 'DueDate': [], 'ServiceTime': [],
            'Q': None, 'C': None, 'r': None, 'g': None, 'v': None}
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in lines[1:]:
            parts = line.strip().split()
            if len(parts) == 0:
                continue
            if parts[0] == 'Q':
                data['Q'] = float(parts[-1].strip('/'))
            elif parts[0] == 'C':
                data['C'] = float(parts[-1].strip('/'))
            elif parts[0] == 'r':
                data['r'] = float(parts[-1].strip('/'))
            elif parts[0] == 'g':
                data['g'] = float(parts[-1].strip('/'))
            elif parts[0] == 'v':
                data['v'] = float(parts[-1].strip('/'))
            elif parts[0].startswith('S'):
                for _ in range(3):  
                    data['StringID'].append(parts[0])
                    data['Type'].append(parts[1])
                    data['x'].append(float(parts[2]))
                    data['y'].append(float(parts[3]))
                    data['Delivery'].append(float(parts[4]))
                    data['Pickup'].append(float(parts[5]))
                    data['ReadyTime'].append(float(parts[6]))
                    data['DueDate'].append(float(parts[7]))
                    data['ServiceTime'].append(float(parts[8]))
            else:
                    data['StringID'].append(parts[0])
                    data['Type'].append(parts[1])
                    data['x'].append(float(parts[2]))
                    data['y'].append(float(parts[3]))
                    data['Delivery'].append(float(parts[4]))
                    data['Pickup'].append(float(parts[5]))
                    data['ReadyTime'].append(float(parts[6]))
                    data['DueDate'].append(float(parts[7]))
                    data['ServiceTime'].append(float(parts[8]))

    return data

def write_processed_file(data, output_file):
    with open(output_file, 'w') as file:
        file.write("Depot_0 = {0};\n")
        file.write("Stations = {")
        for i, s_id in enumerate(data['StringID']):
            if data['Type'][i] == 'f':
                file.write(f"{i},")
        file.write("};\n")
        file.write("Customers = {")
        for i, s_id in enumerate(data['StringID']):
            if data['Type'][i] == 'c':
                file.write(f"{i},")
        file.write("};\n")
        file.write("Depot_N1 = {")
        file.write(f"{len(data['StringID'])}")
        file.write("};\n\n")

        file.write(f"v = {data['v']};\n")
        file.write(f"C = {data['C']};\n")
        file.write(f"Q = {data['Q']};\n")
        file.write(f"g = {data['g']};\n")
        file.write(f"h = {data['r']};\n\n")
        
        file.write("XCoord = #[\n")
        for i, x in enumerate(data['x']):
            file.write(f"{i}\t:\t{x}\n")
        file.write(f"{len(data['x'])}\t:\t{data['x'][0]}\n")  # Add additional line
        file.write("]#;\n\n")
        
        file.write("YCoord = #[\n")
        for i, y in enumerate(data['y']):
            file.write(f"{i}\t:\t{y}\n")
        file.write(f"{len(data['y'])}\t:\t{data['y'][0]}\n")  # Add additional line
        file.write("]#;\n\n")
        
        file.write("q = #[\n")
        for i, delivery in enumerate(data['Delivery']):
            file.write(f"{i}\t:\t{delivery}\n")
        file.write(f"{len(data['Delivery'])}\t:\t{data['Delivery'][0]}\n")  # Add additional line
        file.write("]#;\n\n")

        file.write("p = #[\n")
        for i, pickup in enumerate(data['Pickup']):
            file.write(f"{i}\t:\t{pickup}\n")
        file.write(f"{len(data['Pickup'])}\t:\t{data['Pickup'][0]}\n")  # Add additional line
        file.write("]#;\n\n")
        
        file.write("e = #[\n")
        for i, ready_time in enumerate(data['ReadyTime']):
            file.write(f"{i}\t:\t{ready_time}\n")
        file.write(f"{len(data['ReadyTime'])}\t:\t{data['ReadyTime'][0]}\n")  # Add additional line        
        file.write("]#;\n\n")
        
        file.write("l = #[\n")
        for i, due_date in enumerate(data['DueDate']):
            file.write(f"{i}\t:\t{due_date}\n")
        file.write(f"{len(data['DueDate'])}\t:\t{data['DueDate'][0]}\n")  # Add additional line  
        file.write("]#;\n\n")
        
        file.write("s = #[\n")
        for i, service_time in enumerate(data['ServiceTime']):
            file.write(f"{i}\t:\t{service_time}\n")
        file.write(f"{len(data['ServiceTime'])}\t:\t{data['ServiceTime'][0]}\n")  # Add additional line  
        file.write("]#;\n")

if __name__ == "__main__":
    paths=["./evrptwspd_instances/small_instances/Cplex5er","./evrptwspd_instances/small_instances/Cplex10er","./evrptwspd_instances/small_instances/Cplex15er","./evrptwspd_instances/large_instances"]
    for filePath in paths:
            fcases=os.listdir(filePath) 
            for pcase in fcases:  
                 input_file = filePath+"/"+pcase
                 output_file = filePath.replace("evrptwspd_instances", "cplex_evrptwspd")+"/"+pcase.replace("txt","dat")
                 data = read_instance_file(input_file)
                 write_processed_file(data, output_file)
