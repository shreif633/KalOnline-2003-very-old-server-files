#!/usr/bin/env python3
"""
Final PyGhidra script to decompile KalOnline server binaries with PDB symbols
Uses existing Ghidra projects created by analyzeHeadless
"""

import os
os.environ['GHIDRA_INSTALL_DIR'] = "/workspace/decompile/tools/ghidra_12.1.3_PUBLIC"

import pyghidra
pyghidra.start(False)

from ghidra.framework import ProjectLocator
from ghidra.program.database import ProgramDB
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

def decompile_project(project_name, exe_name, output_dir):
    print(f"\n{'='*60}")
    print(f"Decompiling project: {project_name}")
    print(f"{'='*60}\n")
    
    project_path = File("/workspace/decompile/ghidra_project")
    locator = ProjectLocator(project_path, project_name)
    project = locator.open()
    
    try:
        program_file = project.getRootFolder().getFile(exe_name)
        if not program_file:
            print(f"Program file not found: {exe_name}")
            return
        
        domain_obj = program_file.getDomainObject()
        if not isinstance(domain_obj, ProgramDB):
            print("Not a valid program")
            return
        
        program = domain_obj
        symbol_table = program.getSymbolTable()
        function_manager = program.getFunctionManager()
        
        symbol_count = sum(1 for _ in symbol_table.getAllSymbols(True))
        print(f"Found {symbol_count} symbols from PDB")
        
        decomp_interface = DecompInterface()
        decomp_interface.openProgram(program)
        
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
        
        binary_output_dir = os.path.join(output_dir, f"{exe_name.replace('.exe', '')}_decompiled")
        os.makedirs(binary_output_dir, exist_ok=True)
        
        print(f"Exporting to: {binary_output_dir}")
        print(f"Found {len(class_functions)} classes\n")
        
        total_funcs = sum(len(f) for f in class_functions.values())
        processed = 0
        
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
                sf.write(f"// Decompiled from: {exe_name}\n\n")
                
                monitor = ConsoleTaskMonitor()
                
                for i, func in enumerate(functions):
                    processed += 1
                    if processed % 100 == 0:
                        print(f"Progress: {processed}/{total_funcs} functions")
                    
                    sym = symbol_table.getPrimarySymbol(func.getEntryPoint())
                    func_name = str(sym.getName()) if sym else f"FUN_{func.getEntryPoint()}"
                    
                    results = decomp_interface.decompileFunction(func, 0, monitor)
                    
                    if results.decompileCompleted():
                        decompiled = str(results.getDecompiledFunction().getC())
                        
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
        print(f"\nComplete: {exe_name} - {processed} functions exported")
        
    finally:
        project.close()

def main():
    output_dir = "/workspace/decompile/output"
    
    projects = [
        ("KalOnline_AuthSvrT", "AuthSvrT.exe"),
        ("KalOnline_DBSvrT", "DBSvrT.exe"),
        ("KalOnline_MainSvrT", "MainSvrT.exe"),
    ]
    
    for project_name, exe_name in projects:
        project_path = File(f"/workspace/decompile/ghidra_project/{project_name}.rep")
        if project_path.exists():
            decompile_project(project_name, exe_name, output_dir)
        else:
            print(f"Project not found: {project_name}")
    
    print("\n" + "="*60)
    print("ALL DECOMPILATION COMPLETE!")
    print("="*60)
    
    print("\nGenerated files:")
    cpp_count = 0
    h_count = 0
    for root, dirs, files in os.walk(output_dir):
        for f in sorted(files):
            if f.endswith('.cpp'):
                cpp_count += 1
                print(f"  [CPP] {os.path.join(root, f)}")
            elif f.endswith('.h'):
                h_count += 1
                print(f"  [H]   {os.path.join(root, f)}")
    
    print(f"\nTotal: {h_count} header files, {cpp_count} source files")

if __name__ == "__main__":
    main()
