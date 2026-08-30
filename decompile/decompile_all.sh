#!/bin/bash
# Full Decompilation Script for KalOnline Server Binaries
# Uses Ghidra with PDB files to recover original symbols

set -e

export JAVA_HOME="/opt/jdk-21.0.2"
export PATH="$JAVA_HOME/bin:$PATH"

GHIDRA_HOME="/workspace/decompile/tools/ghidra_12.1.3_PUBLIC"
PROJECT_DIR="/workspace/decompile/ghidra_project"
OUTPUT_DIR="/workspace/decompile/output"
SCRIPTS_DIR="/workspace/decompile/scripts"
BINARIES_DIR="/workspace"

# Binaries to decompile
declare -a EXES=("AuthSvrT.exe" "DBSvrT.exe" "MainSvrT.exe")

echo "=========================================="
echo "KalOnline Server Full Decompilation Plan"
echo "=========================================="
echo ""

# Check Java
if ! command -v java &> /dev/null; then
    echo "ERROR: Java not found. Please install JDK 17+"
    exit 1
fi

java_version=$(java -version 2>&1 | head -1)
echo "Java version: $java_version"

# Create project directory
mkdir -p "$PROJECT_DIR"

echo ""
echo "Step 1: Importing and analyzing binaries with PDB symbols..."
echo ""

for exe in "${EXES[@]}"; do
    echo "----------------------------------------"
    echo "Processing: $exe"
    echo "----------------------------------------"
    
    exe_path="$BINARIES_DIR/$exe"
    pdb_path="$BINARIES_DIR/${exe%.exe}.pdb"
    
    if [ ! -f "$exe_path" ]; then
        echo "WARNING: $exe not found, skipping..."
        continue
    fi
    
    if [ ! -f "$pdb_path" ]; then
        echo "WARNING: PDB file $pdb_path not found, proceeding without full symbols..."
    else
        echo "PDB file found: $pdb_path"
    fi
    
    # Run Ghidra headless analyzer with PDB
    # The -import option loads the binary and its PDB automatically if in same directory
    "$GHIDRA_HOME/support/analyzeHeadless" \
        "$PROJECT_DIR" \
        "KalOnline_${exe%.exe}" \
        -import "$exe_path" \
        -scriptPath "$SCRIPTS_DIR" \
        -postScript DecompileAndExport.java "$OUTPUT_DIR" \
        -deleteProject \
        2>&1 | tee "$OUTPUT_DIR/${exe%.exe}_analysis.log"
    
    echo ""
    echo "Completed analysis for $exe"
    echo "Log saved to: $OUTPUT_DIR/${exe%.exe}_analysis.log"
    echo ""
done

echo "=========================================="
echo "Decompilation Complete!"
echo "=========================================="
echo ""
echo "Output directory: $OUTPUT_DIR"
echo ""
echo "Generated files:"
find "$OUTPUT_DIR" -name "*.cpp" -o -name "*.h" | sort

echo ""
echo "Next steps:"
echo "1. Review generated .h and .cpp files in $OUTPUT_DIR"
echo "2. Fix any syntax errors or missing type definitions"
echo "3. Organize classes into proper project structure"
echo "4. Rebuild and test functionality"
