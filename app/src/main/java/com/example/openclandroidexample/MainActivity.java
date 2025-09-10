package com.example.openclandroidexample;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.util.Log;

import com.google.android.material.textfield.TextInputEditText;

import java.util.Arrays;
import java.util.Random;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class MainActivity extends AppCompatActivity {
    public static final String OPEN_CL_EXAMPLE_LOGS = "OpenCL-EXAMPLE-logs";
    public native String processCommand(String op, float[][] matrix1, float[][] matrix2);
    public native int getgpuinfos();
    public native String dumpOpenCLDevices();
    public native String getGpuName();
    // Load the native C++ library

    static void loadnativelib(){
            //TODO wtf ? OpenCL is /vendor/lib64/libOpenCL.so
            // solved in AndroidManifest.xml
            //System.loadLibrary("OpenCL");
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Loaded successfully");
            System.loadLibrary("openclandroidexample");
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Loaded successfully");

    }

    // Declare native methods
    public String add(float[][] a, float[][] b){
        String result = "???";
        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Adding %s,%s using native", Arrays.deepToString(a), Arrays.deepToString(b)));
        result = processCommand("add",a,b);
        Log.d(OPEN_CL_EXAMPLE_LOGS, "done adding");
        if (getgpuinfos() == 1){
            Log.d(OPEN_CL_EXAMPLE_LOGS,"getgpuinfos failed");
        }
        return result;
    }
    public String sub(float[][] a, float[][] b){
        String result = "???";
        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("subbing %s,%s using native", Arrays.deepToString(a), Arrays.deepToString(b)));
        result = processCommand("sub",a,b);
        Log.d(OPEN_CL_EXAMPLE_LOGS, "done subbing");
        return result;
    }
    public String matrixMultiply(float[][] a, float[][] b){
        String result = "???";
        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("matrixMultiply %s,%s using native", Arrays.deepToString(a), Arrays.deepToString(b)));
        result = processCommand("matmul",a,b);
        Log.d(OPEN_CL_EXAMPLE_LOGS, "done matrixMultiply");
        return result;
    }
    public String transpose(float[][] a){
        String result = "???";
        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("transposing %s using native", Arrays.deepToString(a)));
        result = processCommand("transpose",a, null);
        Log.d(OPEN_CL_EXAMPLE_LOGS, "done transpose");
        return result;
    }


    private TextInputEditText matrixARows, matrixACols, matrixAValues;
    private TextInputEditText matrixBRows, matrixBCols, matrixBValues;

    private Button AddButton;
    private Button MatMulButton;
    private Button SubButton;
    private Button TransposeButton;
    private Button GpuInfoButton;
    private Button ClearValuesButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(OPEN_CL_EXAMPLE_LOGS, "in OnCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize UI components
        matrixARows = findViewById(R.id.matrix_a_rows);
        matrixACols = findViewById(R.id.matrix_a_cols);
        matrixAValues = findViewById(R.id.matrix_a_values);
        matrixBRows = findViewById(R.id.matrix_b_rows);
        matrixBCols = findViewById(R.id.matrix_b_cols);
        matrixBValues = findViewById(R.id.matrix_b_values);

        AddButton = findViewById(R.id.btn_add);
        SubButton = findViewById(R.id.btn_sub);
        MatMulButton = findViewById(R.id.btn_matrix_multiply);
        TransposeButton = findViewById(R.id.btn_transpose);
        GpuInfoButton = findViewById(R.id.btn_gpu_info);
        ClearValuesButton = findViewById(R.id.btn_clear_values);

        AddButton.setOnClickListener(v-> preProcessCommand("add"));
        SubButton.setOnClickListener(v-> preProcessCommand("sub"));
        MatMulButton.setOnClickListener(v-> preProcessCommand("matmul"));
        TransposeButton.setOnClickListener(v-> preProcessCommand("transpose"));
        GpuInfoButton.setOnClickListener(v -> showGpuInfoDialog());
        ClearValuesButton.setOnClickListener(v -> clearMatrixValues());
        Log.d(OPEN_CL_EXAMPLE_LOGS, "OnCreate layout setup done");
        Log.d(OPEN_CL_EXAMPLE_LOGS, "loading native lib");
        loadnativelib();
        Log.d(OPEN_CL_EXAMPLE_LOGS, "native lib loaded successfully");

        try {
            String gpuName = getGpuName();
            if (gpuName != null && !gpuName.isEmpty()) {
                ((android.widget.TextView)findViewById(R.id.device_info_text)).setText("Using Device: " + gpuName);
            }
        } catch (Throwable t) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Failed to fetch GPU name: " + t.getMessage());
        }
    }

    private void clearMatrixValues() {
        try {
            if (matrixAValues != null) matrixAValues.setText("");
            if (matrixBValues != null) matrixBValues.setText("");
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Cleared matrix values");
        } catch (Throwable t) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Failed to clear values: " + t.getMessage());
        }
    }

    private void showGpuInfoDialog() {
        String info = "";
        try {
            info = dumpOpenCLDevices();
        } catch (Throwable t) {
            info = "Failed to dump OpenCL devices: " + t.getMessage();
        }
        new androidx.appcompat.app.AlertDialog.Builder(this)
                .setTitle("OpenCL Devices")
                .setMessage(info)
                .setPositiveButton("OK", (d, w) -> d.dismiss())
                .show();
    }

    private void preProcessCommand(String selectedOp) {
        try {
            Log.d(OPEN_CL_EXAMPLE_LOGS, "in preProcessCommand");
            int aRows = Integer.parseInt( matrixARows.getText().toString());
            int aCols = Integer.parseInt(matrixACols.getText().toString());
            if (aRows < 1 || aRows > 20 || aCols < 1 || aCols > 20) {
                throw new IllegalArgumentException("Rows and columns for Matrix A must be between 1 and 20.");
            }
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("in preProcessCommand : matrix A rows %d, matrix A cols %d", aRows, aCols));
            float[][] matrixA = parseOrGenerateMatrix(matrixAValues, aRows, aCols);
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("in preProcessCommand : parseMatrix(matrix A) done = %s", Arrays.deepToString(matrixA)));

            int bRows = Integer.parseInt(matrixBRows.getText().toString());
            int bCols = Integer.parseInt(matrixBCols.getText().toString());
            if (bRows < 1 || bRows > 20 || bCols < 1 || bCols > 20) {
                throw new IllegalArgumentException("Rows and columns for Matrix B must be between 1 and 20.");
            }
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("in preProcessCommand : matrix B rows %d, matrix B cols %d", aRows, aCols));
            float[][] matrixB = parseOrGenerateMatrix(matrixBValues, bRows, bCols);
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("in preProcessCommand : parseMatrix(matrix B) done = %s", Arrays.deepToString(matrixA)));


            String resultMatrix = "";

            if ("add".equals(selectedOp)) {
                resultMatrix = add(matrixA, matrixB);
            } else if ("sub".equals(selectedOp)) {
                resultMatrix = sub(matrixA, matrixB);
            } else if ("matmul".equals(selectedOp)) {
                resultMatrix = matrixMultiply(matrixA, matrixB);
            } else if ("transpose".equals(selectedOp)) {
                resultMatrix = transpose(matrixA);
            }

            if (resultMatrix == null || resultMatrix.isEmpty() || resultMatrix.equals("???")) {
                throw new IllegalArgumentException("Operation failed or resulted in an empty matrix. Check dimensions.");
            }

            // Start Result Activity with the result
            Intent intent = new Intent(MainActivity.this, ResultActivity.class);
            Bundle bundle = new Bundle();
            bundle.putSerializable("result_matrix", resultMatrix);
            intent.putExtras(bundle);
            startActivity(intent);

        } catch (NullPointerException NPE){
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("NullPointerException detected when ", NPE.getMessage()));
        }
        catch (NumberFormatException e) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("NumberFormatException detected when ", e.getMessage()));
            showError("Invalid number format in rows, columns, or values.");
        } catch (IllegalArgumentException e) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("IllegalArgumentException detected when ", e.getMessage()));
            showError(e.getMessage());
        } catch (Exception e) {
            // This can catch errors from the native code
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Exception detected when ", e.getMessage()));
            showError("An unexpected error occurred: " + e.getMessage());
        }
    }

    private float[][] parseOrGenerateMatrix(TextInputEditText UIid, int rows, int cols) throws IllegalArgumentException {
        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("parseMatrix called with rows=%d, cols=%d", rows, cols));

        float[][] matrix = new float[rows][cols];
        String text =  UIid.getText().toString();

        if (text == null || text.trim().isEmpty()) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, "Input text is empty, generating random matrix");

            Random random = new Random();
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    float value = -20 + random.nextFloat() * 40;   // -20 + [0..40) = [-20..20)
                    value = Math.round(value * 100f) / 100f;    // round to 2 decimals
                    matrix[i][j] = value; // Random float between 0.0 and 1.0
                    Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Generated random value at (%d,%d): %f", i, j, matrix[i][j]));
                }
            }

            // Update UI with generated matrix if needed
            runOnUiThread(() -> {
                StringBuilder sb = new StringBuilder();
                sb.append("[");
                for (int i = 0; i < rows; i++) {
                    sb.append("[");
                    for (int j = 0; j < cols; j++) {
                        sb.append(String.format("%.2f", matrix[i][j]));
                        if (j < cols - 1) sb.append(", ");
                    }
                    sb.append("]");
                    if (i < rows - 1) sb.append(",\n");
                }
                sb.append("]");
                UIid.setText(sb.toString());
            });

            return matrix;
        }

        Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Input text is %s",text));
        // Extract all numeric tokens (handles brackets, newlines, spaces, and commas)
        Pattern numberPattern = Pattern.compile("[-+]?((\\d+(\\.\\d+)?)|(\\.\\d+))([eE][-+]?\\d+)?");
        Matcher matcher = numberPattern.matcher(text);
        List<Float> values = new ArrayList<>();
        while (matcher.find()) {
            try {
                values.add(Float.parseFloat(matcher.group()));
            } catch (NumberFormatException e) {
                // Shouldn't happen due to regex, but guard anyway
                Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Skipping invalid token '%s'", matcher.group()));
            }
        }
        Log.d(OPEN_CL_EXAMPLE_LOGS, "Extracted " + values.size() + " numeric values");

        if (values.size() != rows * cols) {
            Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Mismatch: expected %d values but got %d", rows * cols, values.size()));
            throw new IllegalArgumentException(
                    String.format("Number of values (%d) does not match matrix dimensions (%dx%d)", values.size(), rows, cols)
            );
        }

        int index = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                float value = values.get(index);
                Log.d(OPEN_CL_EXAMPLE_LOGS, String.format("Parsing value at row %d, col %d: '%s'", i, j, Float.toString(value)));
                matrix[i][j] = value;
                index++;
            }
        }

        Log.d(OPEN_CL_EXAMPLE_LOGS, "Matrix parsing successful");
        return matrix;
    }


    //TODO : fix this shit
    private void showError(String message) {
        Intent intent = new Intent(MainActivity.this, ResultActivity.class);
        intent.putExtra("error_message", message);
        startActivity(intent);
    }
}
