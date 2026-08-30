#!/usr/bin/env python3
"""
PyGhidra script to decompile KalOnline server binaries with PDB symbols
and export C++ code with original class names and function names.
"""

import sys
import os
from pathlib import Path

# Initialize PyGhidra first
os.environ['GHIDRA_INSTALL_DIR'] = "/workspace/decompile/tools/ghidra_12.1.3_PUBLIC"
import pyghidra
pyghidra.start(False)  # headless mode

from ghidra.app.decompiler import DecompInterface, DecompileResults
from ghidra.program.flatapi import FlatProgramAPI
from ghidra.util.task import ConsoleTaskMonitor
from java.io import File
from java.util import TreeMap, ArrayList
import re

def extract_class_name(symbol_name):
    """Extract class name from C++ symbol"""
    if "::" in symbol_name:
        parts = symbol_name.split("::")
        if len(parts) >= 2:
            return parts[0].lstrip("@")
    return "Global"

def sanitize_name(name):
    """Make name safe for filenames"""
    return re.sub(r'[^a-zA-Z0-9_]', '_', name)

def extract_declaration(decompiled_code, func_name):
    """Extract function declaration from decompiled code"""
    lines = decompiled_code.split("\n")
    for line in lines:
        line = line.strip()
        if func_name in line and "(" in line:
            if line.endswith(";"):
                return line
            if "{" in line:
                return line[:line.index("{")].strip() + ";"
    return None

def decompile_binary(exe_path, pdb_path, output_dir):
    """Decompile a single binary with PDB"""
    print(f"\n{'='*60}")
    print(f"Decompiling: {exe_path}")
    print(f"PDB: {pdb_path}")
    print(f"{'='*60}\n")
    
    # Create project using Ghidra project API
    project_name = f"KalOnline_{Path(exe_path).stem}"
    project_dir = File("/workspace/decompile/ghidra_project")
    
    # Use FlatProgramAPI to load the program
    from ghidra.program.util import GhidraProgramUtilities
    from ghidra.framework import Project, DomainFile
    from ghidra.framework.model import DomainObject
    
    # Import the program
    from ghidra.app.util import ProgramUtils
    from ghidra.app.util.importer import MessageLog
    from ghidra.framework.plugintool import PluginManager
    
    # Load program directly
    from ghidra.program.database import ProgramDB
    from ghidra.app.util.opinion import ProgramLoader, LoaderService
    
    exe_file = File(exe_path)
    pdb_file = File(pdb_path) if pdb_path and os.path.exists(pdb_path) else None
    
    # Get program from existing Ghidra project
    project_path = File(f"/workspace/decompile/ghidra_project/{project_name}.rep")
    
    if not project_path.exists():
        print(f"Project not found, need to run analyzeHeadless first")
        return
    
    # Open the project and get the program
    from ghidra.framework import ProjectLocator
    locator = ProjectLocator(File("/workspace/decompile/ghidra_project"), project_name)
    project = locator.open()
    
    # Get the program file
    program_file = project.getRootFolder().getFile(Path(exe_path).name)
    if program_file is None:
        print(f"Could not find program in project")
        project.close()
        return
    
    # Get domain object
    domain_obj = program_file.getDomainObject()
    if not isinstance(domain_obj, ProgramDB):
        print(f"Not a valid program object")
        project.close()
        return
    
    program = domain_obj
    
    try:
        symbol_table = program.getSymbolTable()
        function_manager = program.getFunctionManager()
        
        # Count symbols
        symbol_count = sum(1 for _ in symbol_table.getAllSymbols(True))
        print(f"Found {symbol_count} symbols from PDB")
        
        # Setup decompiler
        decomp_interface = DecompInterface()
        decomp_interface.openProgram(program)
        
        # Group functions by class
        class_functions = TreeMap()
        
        for func in function_manager.getFunctions(True):
            class_name = "Global"
            primary_symbol = symbol_table.getPrimarySymbol(func.getEntryPoint())
            
            if primary_symbol:
                sym_name = primary_symbol.getName(True)
                class_name = extract_class_name(sym_name)
            
            if class_name not in class_functions:
                class_functions[class_name] = ArrayList()
            class_functions[class_name].add(func)
        
        # Create output directory for this binary
        binary_output_dir = File(os.path.join(output_dir, f"{Path(exe_path).stem}_decompiled"))
        binary_output_dir.mkdirs()
        
        print(f"Exporting to: {binary_output_dir.getAbsolutePath()}")
        
        # Export each class
        for class_name, functions in class_functions.entrySet():
            header_path = File(binary_output_dir, f"{class_name}.h")
            source_path = File(binary_output_dir, f"{class_name}.cpp")
            
            with open(header_path.absolutePath, 'w') as hf, \
                 open(source_path.absolutePath, 'w') as sf:
                
                # Write header
                hf.write(f"#ifndef {sanitize_name(class_name)}_H\n")
                hf.write(f"#define {sanitize_name(class_name)}_H\n\n")
                hf.write(f"// Class: {class_name}\n")
                hf.write(f"// Functions: {functions.size()}\n\n")
                
                # Write source
                if class_name != "Global":
                    sf.write(f'#include "{class_name}.h"\n\n')
                sf.write(f"// Class: {class_name}\n")
                sf.write(f"// Decompiled from: {Path(exe_path).name}\n\n")
                
                monitor = ConsoleTaskMonitor()
                func_num = 0
                
                for func in functions:
                    func_num += 1
                    if func_num % 50 == 0:
                        print(f"  Processing {class_name}::{func_num}/{functions.size()}")
                    
                    sym = symbol_table.getPrimarySymbol(func.getEntryPoint())
                    func_name = sym.getName() if sym else f"FUN_{func.getEntryPoint()}"
                    
                    # Decompile
                    results = decomp_interface.decompileFunction(func, 0, monitor)
                    
                    if results.decompileCompleted():
                        decompiled = results.getDecompiledFunction().getC()
                        
                        # Extract and write declaration
                        decl = extract_declaration(decompiled, func_name)
                        if decl:
                            hf.write(f"{decl}\n")
                        
                        # Write implementation
                        sf.write(f"// Function: {func_name}\n")
                        sf.write(f"// Address: {func.getEntryPoint()}\n\n")
                        sf.write(decompiled)
                        sf.write("\n" + "="*80 + "\n\n")
                
                hf.write(f"\n#endif // {sanitize_name(class_name)}_H\n")
            
            print(f"  Exported {class_name} ({functions.size()} functions)")
        
        decomp_interface.dispose()
        print(f"\nDecompilation complete for {Path(exe_path).name}")
        
    finally:
        program.release()
        project.close()

def main():
    output_dir = "/workspace/decompile/output"
    binaries_dir = "/workspace"
    
    binaries = [
        ("AuthSvrT.exe", "AuthSvrT.pdb"),
        ("DBSvrT.exe", "DBSvrT.pdb"),
        ("MainSvrT.exe", "MainSvrT.pdb"),
    ]
    
    for exe, pdb in binaries:
        exe_path = os.path.join(binaries_dir, exe)
        pdb_path = os.path.join(binaries_dir, pdb)
        
        if os.path.exists(exe_path):
            decompile_binary(exe_path, pdb_path if os.path.exists(pdb_path) else None, output_dir)
        else:
            print(f"Binary not found: {exe_path}")
    
    print("\n" + "="*60)
    print("All decompilations complete!")
    print("="*60)
    
    # List generated files
    print("\nGenerated files:")
    for root, dirs, files in os.walk(output_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                print(f"  {os.path.join(root, file)}")

if __name__ == "__main__":
    main()
