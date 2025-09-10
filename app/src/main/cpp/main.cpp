#if defined(BUILD_ENGINE_CLI) || defined(BUILD_ENGINE_CLI_DESKTOP)

#include "native-lib.cpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <cstring>  // for memcpy
#include <random>
#include <cmath> // for std::round

using Matrix = std::vector<std::vector<float>>;

Matrix parseOrGenerateMatrix(const std::string& matrixStr, int rows = 0, int cols = 0) {
    std::cout<< "in parseOrgenerateMatrix(" <<matrixStr<<","<<rows<<","<<cols<<")"<<std::endl;
    // If the string is empty, generate a zero matrix with given dimensions
    if (matrixStr.empty()) {
        if (rows <= 0 || cols <= 0) {
            throw std::invalid_argument("Empty matrix string requires valid row and column dimensions");
        }
        // Random number generation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-20.0f, 20.0f);

        Matrix matrix(rows, std::vector<float>(cols));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Round to 2 decimal places
                float val = dist(gen);
                val = std::round(val * 100.0f) / 100.0f;
                matrix[i][j] = val;
            }
        }
        return matrix;
    }

    std::string s = matrixStr;

    if (s.size() < 4 || s.substr(0, 2) != "[[" || s.substr(s.size() - 2) != "]]") {
        throw std::invalid_argument("Invalid matrix format: " + s);
    }

    // Remove outer [[ ]]
    s = s.substr(2, s.size() - 4);

    // Split rows by "],["
    Matrix matrix;
    size_t start = 0;
    while (true) {
        size_t pos = s.find("],[", start);
        std::string rowStr = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);

        std::vector<float> row;
        std::stringstream ss(rowStr);
        std::string val;
        while (std::getline(ss, val, ',')) {
            row.push_back(std::stof(val));
        }
        matrix.push_back(row);

        if (pos == std::string::npos) break;
        start = pos + 3; // skip "],["
    }

    return matrix;
}

std::string processCommand(
        const std::string& operation,
        const Matrix& mat1,
        const Matrix& mat2)
{

    if (kernel_map.find(operation) == kernel_map.end()) {
        return "Operation not supported.";
    }

    OpenCLManager ocl;
    if (!ocl.init()) {
        return "Failed to initialize OpenCL";
    }

    KernelInfo k_info = kernel_map[operation];
    cl_program program = ocl.createProgram(k_info.source);
    cl_kernel kernel = ocl.createKernel(program, k_info.name);

    std::string result_str;

    if (operation == "add" || operation == "sub" || operation == "mul") {
        if (mat1.empty() || mat2.empty() ||
            mat1.size() != mat2.size() ||
            mat1[0].size() != mat2[0].size()) {
            return "Matrices must have the same dimensions for this operation.";
        }

        int rows = mat1.size();
        int cols = mat1[0].size();
        int num_elements = rows * cols;

        std::vector<float> flat_mat1(num_elements), flat_mat2(num_elements), result_mat(num_elements);
        for (int i = 0; i < rows; ++i) {
            memcpy(flat_mat1.data() + i * cols, mat1[i].data(), cols * sizeof(float));
            memcpy(flat_mat2.data() + i * cols, mat2[i].data(), cols * sizeof(float));
        }

        cl_mem bufA = ocl.createBuffer(sizeof(float) * num_elements, CL_MEM_READ_WRITE);
        cl_mem bufB = ocl.createBuffer(sizeof(float) * num_elements, CL_MEM_READ_WRITE);
        cl_mem bufC = ocl.createBuffer(sizeof(float) * num_elements, CL_MEM_READ_WRITE);
        ocl.writeBuffer(bufA, sizeof(float) * num_elements, flat_mat1.data());
        ocl.writeBuffer(bufB, sizeof(float) * num_elements, flat_mat2.data());

        ocl.setKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
        ocl.setKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
        ocl.setKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
        ocl.setKernelArg(kernel, 3, sizeof(int), &num_elements);

        size_t global_work_size = num_elements;
        ocl.executeKernel(kernel, 1, &global_work_size);
        ocl.readBuffer(bufC, sizeof(float) * num_elements, result_mat.data());

        result_str = format_matrix(result_mat, rows, cols);

        ocl.releaseMemObject(bufA);
        ocl.releaseMemObject(bufB);
        ocl.releaseMemObject(bufC);
    }
    else if (operation == "transpose") {
        if (mat1.empty()) {
            return "Matrix cannot be empty for transpose.";
        }

        int rows = mat1.size();
        int cols = mat1[0].size();

        std::vector<float> flat_mat1(rows * cols), result_mat(rows * cols);
        for (int i = 0; i < rows; ++i) {
            memcpy(flat_mat1.data() + i * cols, mat1[i].data(), cols * sizeof(float));
        }

        cl_mem bufA = ocl.createBuffer(sizeof(float) * rows * cols, CL_MEM_READ_WRITE);
        cl_mem bufC = ocl.createBuffer(sizeof(float) * rows * cols, CL_MEM_READ_WRITE);
        ocl.writeBuffer(bufA, sizeof(float) * rows * cols, flat_mat1.data());

        ocl.setKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
        ocl.setKernelArg(kernel, 1, sizeof(cl_mem), &bufC);
        ocl.setKernelArg(kernel, 2, sizeof(int), &rows);
        ocl.setKernelArg(kernel, 3, sizeof(int), &cols);

        size_t global_work_size[2] = { (size_t)cols, (size_t)rows };
        ocl.executeKernel(kernel, 2, global_work_size);
        ocl.readBuffer(bufC, sizeof(float) * rows * cols, result_mat.data());

        result_str = format_matrix(result_mat, cols, rows);

        ocl.releaseMemObject(bufA);
        ocl.releaseMemObject(bufC);
    }
    else if (operation == "matmul") {
        if (mat1.empty() || mat2.empty() || mat1[0].size() != mat2.size()) {
            return "Invalid dimensions for matrix multiplication";
        }

        int M = mat1.size();
        int N = mat1[0].size();
        int K = mat2[0].size();

        std::vector<float> flat_mat1(M * N), flat_mat2(N * K), result_mat(M * K);
        for (int i = 0; i < M; ++i) {
            memcpy(flat_mat1.data() + i * N, mat1[i].data(), N * sizeof(float));
        }
        for (int i = 0; i < N; ++i) {
            memcpy(flat_mat2.data() + i * K, mat2[i].data(), K * sizeof(float));
        }

        cl_mem bufA = ocl.createBuffer(sizeof(float) * M * N, CL_MEM_READ_WRITE);
        cl_mem bufB = ocl.createBuffer(sizeof(float) * N * K, CL_MEM_READ_WRITE);
        cl_mem bufC = ocl.createBuffer(sizeof(float) * M * K, CL_MEM_READ_WRITE);
        ocl.writeBuffer(bufA, sizeof(float) * M * N, flat_mat1.data());
        ocl.writeBuffer(bufB, sizeof(float) * N * K, flat_mat2.data());

        ocl.setKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
        ocl.setKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
        ocl.setKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
        ocl.setKernelArg(kernel, 3, sizeof(int), &M);
        ocl.setKernelArg(kernel, 4, sizeof(int), &N);
        ocl.setKernelArg(kernel, 5, sizeof(int), &K);

        size_t global_work_size[2] = { (size_t)M, (size_t)K };
        ocl.executeKernel(kernel, 2, global_work_size);
        ocl.readBuffer(bufC, sizeof(float) * M * K, result_mat.data());

        result_str = format_matrix(result_mat, M, K);

        ocl.releaseMemObject(bufA);
        ocl.releaseMemObject(bufB);
        ocl.releaseMemObject(bufC);
    }

    ocl.releaseKernel(kernel);
    ocl.releaseProgram(program);

    return result_str;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: <op> <matrix1> [<matrix2>] or <op> (cols,rows) (cols,rows)" << std::endl;
        return 1;
    }

    std::string operation = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    Matrix matrix1, matrix2;

    try {
        // Detect dimension mode: args start with '('
        std::regex dimRegex("\\((\\d+),(\\d+)\\)");
        std::smatch match;

        if (!args.empty() && std::regex_search(args[0], match, dimRegex)) {
            // Dimension mode
            int cols1 = std::stoi(match[1]);
            int rows1 = std::stoi(match[2]);
            matrix1 = parseOrGenerateMatrix("", rows1, cols1); // auto-generated random matrix

            if (args.size() > 1) {
                if (!std::regex_match(args[1], match, dimRegex)) {
                    throw std::invalid_argument("Invalid dimensions for matrix2: " + args[1]);
                }
                int cols2 = std::stoi(match[1]);
                int rows2 = std::stoi(match[2]);
                matrix2 = parseOrGenerateMatrix("", rows2, cols2); // auto-generated random matrix
            }
        } else {
            // Matrix string mode
            // Join all args to single string for regex split
            std::string commandStr;
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) commandStr += " ";
                commandStr += args[i];
            }

            // Split into matrix parts using "[[" as row start
            std::regex re(" (?=\\[\\[)");
            std::sregex_token_iterator it(commandStr.begin(), commandStr.end(), re, -1);
            std::sregex_token_iterator end;
            std::vector<std::string> parts(it, end);

            if (parts.size() < 1) {
                throw std::invalid_argument("No matrix specified");
            }

            matrix1 = parseOrGenerateMatrix(parts[0]);
            if (parts.size() > 1) {
                matrix2 = parseOrGenerateMatrix(parts[1]);
            }
        }

        std::cout << "Operation is {" << operation << "}" << std::endl;
        std::cout << "calling processCommand" << std::endl;
        std::string result = processCommand(operation, matrix1, matrix2);
        std::cout << "processCommand success" << std::endl;
        std::cout << "result: " << result << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error processing command: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "dumping infos" << std::endl;
    auto opencl_infos = getOpenCLDevicesInfoString();
    std::cout << opencl_infos << std::endl;
    return 0;
}


#endif

