#!/usr/bin/env python3
"""
Simple PyGhidra script to decompile KalOnline server binaries with PDB symbols
"""

import os
os.environ['GHIDRA_INSTALL_DIR'] = "/workspace/decompile/tools/ghidra_12.1.3_PUBLIC"

import pyghidra
pyghidra.start(False)

from ghidra.program.flatapi import FlatProgramAPI
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
from java.io import File
import re

def extract_class_name(symbol_name):
    if "::" in symbol_name:
        parts = symbol_name.split("::")
        if len(parts) >= 2:
            return parts[0].lstrip("@")
    return "Global"

def sanitize_name(name):
    return re.sub(r'[^a-zA-Z0-9_]', '_', name)

def decompile_binary(exe_path, output_dir):
    print(f"\n{'='*60}")
    print(f"Decompiling: {exe_path}")
    print(f"{'='*60}\n")
    
    exe_file = File(exe_path)
    program = FlatProgramAPI.openProgram(exe_file, None, False)
    
    try:
        symbol_table = program.getSymbolTable()
        function_manager = program.getFunctionManager()
        
        symbol_count = sum(1 for _ in symbol_table.getAllSymbols(True))
        print(f"Found {symbol_count} symbols from PDB")
        
        decomp_interface = DecompInterface()
        decomp_interface.openProgram(program)
        
        # Group functions by class
        class_functions = {}
        
        for func in function_manager.getFunctions(True):
            class_name = "Global"
            primary_symbol = symbol_table.getPrimarySymbol(func.getEntryPoint())
            
            if primary_symbol:
                sym_name = str(primary_symbol.getName(True))
                class_name = extract_class_name(sym_name)
            
            if class_name not in class_functions:
                class_functions[class_name] = []
            class_functions[class_name].append(func)
        
        binary_output_dir = os.path.join(output_dir, f"{os.path.basename(exe_path).replace('.exe', '')}_decompiled")
        os.makedirs(binary_output_dir, exist_ok=True)
        
        print(f"Exporting to: {binary_output_dir}")
        print(f"Found {len(class_functions)} classes")
        
        for class_name, functions in sorted(class_functions.items()):
            header_path = os.path.join(binary_output_dir, f"{sanitize_name(class_name)}.h")
            source_path = os.path.join(binary_output_dir, f"{sanitize_name(class_name)}.cpp")
            
            with open(header_path, 'w') as hf, open(source_path, 'w') as sf:
                hf.write(f"#ifndef {sanitize_name(class_name)}_H\n")
                hf.write(f"#define {sanitize_name(class_name)}_H\n\n")
                hf.write(f"// Class: {class_name}\n")
                hf.write(f"// Functions: {len(functions)}\n\n")
                
                if class_name != "Global":
                    sf.write(f'#include "{sanitize_name(class_name)}.h"\n\n')
                sf.write(f"// Class: {class_name}\n")
                sf.write(f"// Decompiled from: {os.path.basename(exe_path)}\n\n")
                
                monitor = ConsoleTaskMonitor()
                
                for i, func in enumerate(functions):
                    if (i + 1) % 50 == 0:
                        print(f"  Processing {class_name}: {i+1}/{len(functions)}")
                    
                    sym = symbol_table.getPrimarySymbol(func.getEntryPoint())
                    func_name = str(sym.getName()) if sym else f"FUN_{func.getEntryPoint()}"
                    
                    results = decomp_interface.decompileFunction(func, 0, monitor)
                    
                    if results.decompileCompleted():
                        decompiled = str(results.getDecompiledFunction().getC())
                        
                        # Extract declaration
                        for line in decompiled.split("\n"):
                            line = line.strip()
                            if func_name in line and "(" in line:
                                if line.endswith(";"):
                                    hf.write(f"{line}\n")
                                    break
                                if "{" in line:
                                    hf.write(f"{line[:line.index('{')].strip()};\n")
                                    break
                        
                        sf.write(f"// Function: {func_name}\n")
                        sf.write(f"// Address: {func.getEntryPoint()}\n\n")
                        sf.write(decompiled)
                        sf.write("\n" + "="*80 + "\n\n")
                
                hf.write(f"\n#endif // {sanitize_name(class_name)}_H\n")
            
            print(f"  Exported {class_name} ({len(functions)} functions)")
        
        decomp_interface.dispose()
        print(f"\nComplete: {os.path.basename(exe_path)}")
        
    finally:
        program.dispose()

def main():
    output_dir = "/workspace/decompile/output"
    binaries_dir = "/workspace"
    
    binaries = ["AuthSvrT.exe", "DBSvrT.exe", "MainSvrT.exe"]
    
    for exe in binaries:
        exe_path = os.path.join(binaries_dir, exe)
        if os.path.exists(exe_path):
            decompile_binary(exe_path, output_dir)
        else:
            print(f"Not found: {exe_path}")
    
    print("\n" + "="*60)
    print("All done!")
    print("="*60)
    
    print("\nGenerated files:")
    for root, dirs, files in os.walk(output_dir):
        for f in sorted(files):
            if f.endswith(('.cpp', '.h')):
                print(f"  {os.path.join(root, f)}")

if __name__ == "__main__":
    main()
