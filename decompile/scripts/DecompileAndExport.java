// DecompileAndExport.java
// Ghidra Headless Script to decompile binaries with PDB and export C++ code

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.util.task.ConsoleTaskMonitor;
import java.io.*;
import java.util.*;

public class DecompileAndExport extends GhidraScript {
    
    @Override
    public void run() throws Exception {
        println("Starting decompilation with PDB symbols...");
        
        Program program = currentProgram;
        String programName = program.getName();
        SymbolTable symbolTable = program.getSymbolTable();
        
        // Count symbols from PDB
        int symbolCount = 0;
        SymbolIterator allSymbols = symbolTable.getAllSymbols(true);
        while (allSymbols.hasNext()) {
            allSymbols.next();
            symbolCount++;
        }
        println("Found " + symbolCount + " symbols from PDB for: " + programName);
        
        // Setup decompiler
        DecompInterface decompInterface = new DecompInterface();
        decompInterface.setOptions(program);
        decompInterface.openProgram(program);
        
        // Get output directory from script args or use default
        String outputDir = "/workspace/decompile/output";
        if (getScriptArgs().length > 0) {
            outputDir = getScriptArgs()[0];
        }
        
        File outDir = new File(outputDir + "/" + programName.replace(".exe", "") + "_decompiled");
        outDir.mkdirs();
        
        println("Exporting decompiled functions to: " + outDir.getAbsolutePath());
        
        // Group functions by class (from PDB symbols)
        Map<String, List<Function>> classFunctions = new TreeMap<>();
        FunctionManager funcMgr = program.getFunctionManager();
        
        for (Function func : funcMgr.getFunctions(true)) {
            String className = "Global";
            Symbol primarySymbol = symbolTable.getPrimarySymbol(func.getEntryPoint());
            
            if (primarySymbol != null) {
                String symName = primarySymbol.getName(true); // Include namespace
                if (symName.contains("::")) {
                    String[] parts = symName.split("::");
                    if (parts.length >= 2) {
                        className = parts[0].replaceFirst("^@", "");
                    }
                } else if (symName.startsWith("_ZN")) {
                    // Demangle C++ name if needed
                    className = extractClassName(symName);
                }
            }
            
            classFunctions.computeIfAbsent(className, k -> new ArrayList<>()).add(func);
        }
        
        // Export each class to separate files
        for (Map.Entry<String, List<Function>> entry : classFunctions.entrySet()) {
            String className = entry.getKey();
            List<Function> functions = entry.getValue();
            
            File headerFile = new File(outDir, className + ".h");
            File sourceFile = new File(outDir, className + ".cpp");
            
            PrintWriter headerWriter = new PrintWriter(new FileWriter(headerFile));
            PrintWriter sourceWriter = new PrintWriter(new FileWriter(sourceFile));
            
            headerWriter.println("#ifndef " + sanitizeName(className) + "_H");
            headerWriter.println("#define " + sanitizeName(className) + "_H");
            headerWriter.println();
            headerWriter.println("// Class: " + className);
            headerWriter.println("// Functions: " + functions.size());
            headerWriter.println();
            
            if (!className.equals("Global")) {
                sourceWriter.println("#include \"" + className + ".h\"");
                sourceWriter.println();
            }
            sourceWriter.println("// Class: " + className);
            sourceWriter.println("// Decompiled from: " + programName);
            sourceWriter.println();
            
            int funcNum = 0;
            for (Function func : functions) {
                funcNum++;
                if (funcNum % 50 == 0) {
                    println("Processing " + className + ":: function " + funcNum + "/" + functions.size());
                }
                
                Symbol sym = symbolTable.getPrimarySymbol(func.getEntryPoint());
                String funcName = sym != null ? sym.getName() : "FUN_" + func.getEntryPoint().toString();
                
                // Decompile function
                ConsoleTaskMonitor monitor = new ConsoleTaskMonitor();
                DecompileResults results = decompInterface.decompileFunction(func, 0, monitor);
                
                if (results.decompileCompleted()) {
                    String decompiledCode = results.getDecompiledFunction().getC();
                    
                    // Write declaration to header
                    String declaration = extractDeclaration(decompiledCode, funcName);
                    if (declaration != null && !declaration.isEmpty()) {
                        headerWriter.println(declaration + ";");
                    }
                    
                    // Write implementation to source
                    sourceWriter.println("// Function: " + funcName);
                    sourceWriter.println("// Address: " + func.getEntryPoint().toString());
                    sourceWriter.println();
                    sourceWriter.println(decompiledCode);
                    sourceWriter.println("\n" + ("=".repeat(80)) + "\n");
                }
            }
            
            headerWriter.println("\n#endif // " + sanitizeName(className) + "_H");
            
            headerWriter.close();
            sourceWriter.close();
            
            println("Exported " + className + " (" + functions.size() + " functions)");
        }
        
        decompInterface.dispose();
        println("Decompilation complete for " + programName);
    }
    
    private String extractClassName(String mangledName) {
        // Simple extraction - can be enhanced
        if (mangledName.contains("::")) {
            return mangledName.split("::")[0];
        }
        return "Unknown";
    }
    
    private String sanitizeName(String name) {
        return name.replaceAll("[^a-zA-Z0-9_]", "_");
    }
    
    private String extractDeclaration(String decompiledCode, String funcName) {
        // Extract function declaration from decompiled code
        String[] lines = decompiledCode.split("\n");
        for (String line : lines) {
            line = line.trim();
            if (line.contains(funcName) && line.contains("(") && line.endsWith(";")) {
                return line;
            }
            if (line.contains(funcName) && line.contains("(") && line.contains("{")) {
                return line.substring(0, line.indexOf("{")).trim() + ";";
            }
        }
        return null;
    }
}
