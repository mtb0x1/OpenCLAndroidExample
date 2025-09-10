#if !defined(BUILD_ENGINE_CLI_DESKTOP)
#include <jni.h>
#endif
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <cstdio>
#include <cstring>
#if !defined(BUILD_ENGINE_CLI_DESKTOP)
#include <android/log.h>
#endif
#define CL_TARGET_OPENCL_VERSION 120
#include "OpenCL/CL/cl.h"

#define TAG "OpenCL-EXAMPLE-logs"

#if !defined(BUILD_ENGINE_CLI_DESKTOP)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
#include <cstdio>
    #define LOGD(...) printf("DEBUG: " TAG " " __VA_ARGS__); printf("\n")
    #define LOGE(...) printf("ERROR: " TAG " " __VA_ARGS__); printf("\n")
#endif


int getgpuinfos(){
    cl_ulong global_mem = 0;
    cl_ulong local_mem = 0;
    cl_ulong max_alloc = 0;
    cl_platform_id platform_id = nullptr;
    cl_device_id device = nullptr;

    cl_int err;
    err = clGetPlatformIDs(1, &platform_id, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE get_gpu_infos Failed to get platform ID"); return 1; }
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE get_gpu_infos failed to get device ID"); return 1; }

    err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem), &global_mem, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_SIZE) failed"); return 1; }
    err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem), &local_mem, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_SIZE) failed"); return 1; }
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_alloc), &max_alloc, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE clGetDeviceInfo(CL_DEVICE_GLOBAL_MEM_SIZE) failed"); return 1; }

    LOGD("CPPENGINE Global memory: %lu MB", (unsigned long)(global_mem / (1024*1024)));
    LOGD("CPPENGINE Local memory: %lu KB", (unsigned long)(local_mem / 1024));
    LOGD("CPPENGINE Max single buffer alloc: %lu MB", (unsigned long)(max_alloc / (1024*1024)));
    return 0;
}
 
#if !defined(BUILD_ENGINE_CLI_DESKTOP)
//TODO: return value to display them on app
extern "C" JNIEXPORT jint JNICALL
Java_com_example_openclandroidexample_MainActivity_getgpuinfos(JNIEnv* env, jobject){
    return getgpuinfos();
}
#endif





// Helper to query a string property
std::string getStringInfo(cl_device_id device, cl_device_info param) {
    size_t size = 0;
    clGetDeviceInfo(device, param, 0, nullptr, &size);
    std::vector<char> buf(size);
    clGetDeviceInfo(device, param, size, buf.data(), nullptr);
    return std::string(buf.data());
}

// Helper to query a numeric property
template<typename T>
T getNumericInfo(cl_device_id device, cl_device_info param) {
    T value;
    clGetDeviceInfo(device, param, sizeof(T), &value, nullptr);
    return value;
}

std::string getOpenCLDevicesInfoString() {
    std::ostringstream oss;
    cl_uint platformCount = 0;
    clGetPlatformIDs(0, nullptr, &platformCount);

    std::vector<cl_platform_id> platforms(platformCount);
    clGetPlatformIDs(platformCount, platforms.data(), nullptr);

    for (cl_uint i = 0; i < platformCount; i++) {
        char platformName[256];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platformName), platformName, nullptr);
        oss << "Platform " << i << ": " << platformName << '\n';

        cl_uint deviceCount = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);

        std::vector<cl_device_id> devices(deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices.data(), nullptr);

        for (cl_uint j = 0; j < deviceCount; j++) {
            oss << "  Device " << j << ": " << getStringInfo(devices[j], CL_DEVICE_NAME) << '\n';
            oss << "    Vendor: " << getStringInfo(devices[j], CL_DEVICE_VENDOR) << '\n';
            oss << "    Version: " << getStringInfo(devices[j], CL_DEVICE_VERSION) << '\n';
            oss << "    Driver version: " << getStringInfo(devices[j], CL_DRIVER_VERSION) << '\n';
            oss << "    OpenCL C version: " << getStringInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION) << '\n';

            cl_device_type type = getNumericInfo<cl_device_type>(devices[j], CL_DEVICE_TYPE);
            oss << "    Type: ";
            if (type & CL_DEVICE_TYPE_CPU) oss << "CPU ";
            if (type & CL_DEVICE_TYPE_GPU) oss << "GPU ";
            if (type & CL_DEVICE_TYPE_ACCELERATOR) oss << "Accelerator ";
            if (type & CL_DEVICE_TYPE_DEFAULT) oss << "Default ";
            oss << '\n';

            oss << "    Max compute units: " << getNumericInfo<cl_uint>(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS) << '\n';
            oss << "    Max clock frequency: " << getNumericInfo<cl_uint>(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY) << " MHz" << '\n';
            oss << "    Global memory size: " << getNumericInfo<cl_ulong>(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE) / (1024 * 1024) << " MB" << '\n';
            oss << "    Local memory size: " << getNumericInfo<cl_ulong>(devices[j], CL_DEVICE_LOCAL_MEM_SIZE) / 1024 << " KB" << '\n';
            oss << "    Max mem alloc size: " << getNumericInfo<cl_ulong>(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE) / (1024 * 1024) << " MB" << '\n';
            oss << "    Max work-group size: " << getNumericInfo<size_t>(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE) << '\n';

            cl_uint dims = getNumericInfo<cl_uint>(devices[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            std::vector<size_t> maxWorkItemSizes(dims);
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * dims, maxWorkItemSizes.data(), nullptr);
            oss << "    Max work-item sizes: ";
            for (auto s : maxWorkItemSizes) oss << s << ' ';
            oss << '\n';
        }
    }
    return oss.str();
}

// --- OpenCL Manager Class (same as before) ---
class OpenCLManager {
public:
    OpenCLManager();
    ~OpenCLManager();

    bool init();
    cl_program createProgram(const char* source);
    cl_kernel createKernel(cl_program program, const char* kernelName);
    cl_mem createBuffer(size_t size, cl_mem_flags flags);
    void writeBuffer(cl_mem buffer, size_t size, void* data);
    void readBuffer(cl_mem buffer, size_t size, void* data);
    void setKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value);
    void executeKernel(cl_kernel kernel, cl_uint work_dim, const size_t* global_work_size);
    void releaseMemObject(cl_mem mem);
    void releaseKernel(cl_kernel kernel);
    void releaseProgram(cl_program program);

private:
    cl_platform_id platform_id = nullptr;
    cl_device_id device_id = nullptr;
    cl_context context = nullptr;
    cl_command_queue command_queue = nullptr;
};

OpenCLManager::OpenCLManager() = default;
OpenCLManager::~OpenCLManager() {
    if (command_queue) clReleaseCommandQueue(command_queue);
    if (context) clReleaseContext(context);
}

bool OpenCLManager::init() {
    cl_int err;
    err = clGetPlatformIDs(1, &platform_id, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE init Failed to get platform ID"); return false; }
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE init failed to get device ID"); return false; }
    context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE init Failed to create context"); return false; }
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE init Failed to create command queue"); return false; }
    char platformName[256];
    clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(platformName), platformName, nullptr);
    char deviceName[256];
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr);
    LOGD("CPPENGINE Selected Platform %s", platformName);
    LOGD("CPPENGINE Selected Device %s", deviceName);
    return true;
}

cl_program OpenCLManager::createProgram(const char* source) {
    LOGD("CPPENGINE in createProgram");
    cl_int err;
    cl_program program = clCreateProgramWithSource(context, 1, &source, nullptr, &err);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE createProgram Failed to create program"); return nullptr; }
    err = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        LOGE("CPPENGINE createProgram Failed to build program (err=%d)", (int)err);
        // Retrieve and print build log
        size_t log_size = 0;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        if (log_size > 1) {
            std::vector<char> build_log(log_size);
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log.data(), nullptr);
            LOGE("CPPENGINE Build Log:\n%.*s", (int)build_log.size(), build_log.data());
        }
        clReleaseProgram(program);
        return nullptr;
    }
    return program;
}

cl_kernel OpenCLManager::createKernel(cl_program program, const char* kernelName) {
    LOGD("CPPENGINE in createKernel");
    cl_int err;
    cl_kernel kernel = clCreateKernel(program, kernelName, &err);
    if (err != CL_SUCCESS) { LOGE("CPPENGINE createKernel Failed to create kernel"); return nullptr; }
    return kernel;
}

cl_mem OpenCLManager::createBuffer(size_t size, cl_mem_flags flags) {
    LOGD("CPPENGINE createBuffer(size=%zu, flags=0x%llx)", size, static_cast<unsigned long long>(flags));
    cl_int err;
    cl_mem buffer = clCreateBuffer(context, flags, size, nullptr, &err);
    if (err != CL_SUCCESS) { LOGE("Failed to create buffer"); return nullptr; }
    return buffer;
}

void OpenCLManager::writeBuffer(cl_mem buffer, size_t size, void* data) {
    LOGD("CPPENGINE in writeBuffer");
    clEnqueueWriteBuffer(command_queue, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
}

void OpenCLManager::readBuffer(cl_mem buffer, size_t size, void* data) {
    LOGD("CPPENGINE in readBuffer");
    clEnqueueReadBuffer(command_queue, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
}

void OpenCLManager::setKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value) {
    LOGD("CPPENGINE in setKernelArg");
    clSetKernelArg(kernel, arg_index, arg_size, arg_value);
}

void OpenCLManager::executeKernel(cl_kernel kernel, cl_uint work_dim, const size_t* global_work_size) {
    LOGD("CPPENGINE in executeKernel");
    clEnqueueNDRangeKernel(command_queue, kernel, work_dim, nullptr, global_work_size, nullptr, 0, nullptr, nullptr);
    clFinish(command_queue);
}

void OpenCLManager::releaseMemObject(cl_mem mem) { if (mem) clReleaseMemObject(mem); }
void OpenCLManager::releaseKernel(cl_kernel kernel) { if (kernel) clReleaseKernel(kernel); }
void OpenCLManager::releaseProgram(cl_program program) { if (program) clReleaseProgram(program); }

// --- JNI Implementation ---
#if !defined(BUILD_ENGINE_CLI_DESKTOP)
// Helper function to convert jobjectArray (float[][]) to std::vector<std::vector<float>>
std::vector<std::vector<float>> convert_jfloat_array_to_vector(JNIEnv *env, jobjectArray javaArray) {
    LOGD("CPPENGINE in array_to_vector");
    std::vector<std::vector<float>> vec;
    if (javaArray == nullptr) return vec;
    jsize rows = env->GetArrayLength(javaArray);
    vec.resize(rows);
    for (jsize i = 0; i < rows; i++) {
        jfloatArray row = (jfloatArray)env->GetObjectArrayElement(javaArray, i);
        if (row == nullptr) continue;
        jsize cols = env->GetArrayLength(row);
        jfloat *elements = env->GetFloatArrayElements(row, nullptr);
        if (elements == nullptr) continue;
        vec[i].assign(elements, elements + cols);
        env->ReleaseFloatArrayElements(row, elements, 0);
        env->DeleteLocalRef(row);
    }
    return vec;
}
#endif
// --- Kernel Sources ---
const char* add_kernel_source = R"(__kernel void add(const __global float* A, const __global float* B, __global float* C, const int num_elements) { int i = get_global_id(0); if (i < num_elements) { C[i] = A[i] + B[i]; } })";
const char* sub_kernel_source = R"(__kernel void sub(const __global float* A, const __global float* B, __global float* C, const int num_elements) { int i = get_global_id(0); if (i < num_elements) { C[i] = A[i] - B[i]; } })";
const char* transpose_kernel_source = R"(__kernel void transpose(const __global float* A, __global float* C, const int rows, const int cols) { int i = get_global_id(0); int j = get_global_id(1); if (i < cols && j < rows) { C[i * rows + j] = A[j * cols + i]; } })";
const char* matmul_kernel_source = R"(__kernel void matmul(const __global float* A, const __global float* B, __global float* C, const int M, const int N, const int K) { int i = get_global_id(0); int j = get_global_id(1); if (i < M && j < K) { float sum = 0.0f; for (int l = 0; l < N; l++) { sum += A[i * N + l] * B[l * K + j]; } C[i * K + j] = sum; } })";

struct KernelInfo { const char* source; const char* name; };
std::map<std::string, KernelInfo> kernel_map = {
        {"add", {add_kernel_source, "add"}},
        {"sub", {sub_kernel_source, "sub"}},
        {"transpose", {transpose_kernel_source, "transpose"}},
        {"matmul", {matmul_kernel_source, "matmul"}}
};

// Helper to format matrix to string
std::string format_matrix(const std::vector<float>& data, int rows, int cols) {
    // Rough estimate: 12 chars per float + commas/brackets
    std::vector<char> buffer;
    buffer.reserve(data.size() * 12 + rows * 2 + cols * 2);

    buffer.push_back('[');

    for (int i = 0; i < rows; ++i) {
        buffer.push_back('[');
        for (int j = 0; j < cols; ++j) {
            char num_buf[16];
            int len = std::snprintf(num_buf, sizeof(num_buf), "%g", data[i * cols + j]);
            buffer.insert(buffer.end(), num_buf, num_buf + len);

            if (j < cols - 1) buffer.push_back(',');
        }
        buffer.push_back(']');
        if (i < rows - 1) buffer.push_back(',');
    }

    buffer.push_back(']');

    return std::string(buffer.data(), buffer.size());
}
#if !defined(BUILD_ENGINE_CLI_DESKTOP)
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_openclandroidexample_MainActivity_processCommand(
        JNIEnv* env, jobject, jstring op, jobjectArray matrix1, jobjectArray matrix2) {
    LOGD("CPPENGINE processCommand Create kernel with program source =%s", add_kernel_source);
    const char *op_str = env->GetStringUTFChars(op, 0);
    std::string operation(op_str);
    env->ReleaseStringUTFChars(op, op_str);

    if (kernel_map.find(operation) == kernel_map.end()) {
        return env->NewStringUTF("Operation not supported.");
    }

    std::vector<std::vector<float>> mat1 = convert_jfloat_array_to_vector(env, matrix1);
    std::vector<std::vector<float>> mat2 = convert_jfloat_array_to_vector(env, matrix2);

    OpenCLManager ocl;
    if (!ocl.init()) return env->NewStringUTF("Failed to initialize OpenCL");

    KernelInfo k_info = kernel_map[operation];
    cl_program program = ocl.createProgram(k_info.source);
    if (program == nullptr) {
        return env->NewStringUTF("Failed to build OpenCL program. See logs.");
    }
    cl_kernel kernel = ocl.createKernel(program, k_info.name);
    if (kernel == nullptr) {
        ocl.releaseProgram(program);
        return env->NewStringUTF("Failed to create OpenCL kernel.");
    }

    LOGD("CPPENGINE processCommand operation=%s kernel=%s", operation.c_str(), k_info.name);
    std::string result_str;

    if (operation == "add" || operation == "sub") {
        if (mat1.empty() || mat2.empty() || mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
            return env->NewStringUTF("Matrices must have the same dimensions for this operation.");
        }
        int rows = mat1.size();
        int cols = mat1[0].size();
        int num_elements = rows * cols;
        std::vector<float> flat_mat1(num_elements), flat_mat2(num_elements), result_mat(num_elements);
        for(int i=0; i<rows; ++i) memcpy(flat_mat1.data() + i*cols, mat1[i].data(), cols*sizeof(float));
        for(int i=0; i<rows; ++i) memcpy(flat_mat2.data() + i*cols, mat2[i].data(), cols*sizeof(float));

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

        for (size_t i = 0; i < result_mat.size(); ++i) {
            LOGD("raw result_mat[%zu] = %f", i, result_mat[i]);
        }

        result_str = format_matrix(result_mat, rows, cols);
        LOGD("string result_mat = %s",result_str.c_str());

        ocl.releaseMemObject(bufA); ocl.releaseMemObject(bufB); ocl.releaseMemObject(bufC);
    } else if (operation == "transpose") {
        if (mat1.empty()) return env->NewStringUTF("Matrix cannot be empty for transpose.");
        int rows = mat1.size();
        int cols = mat1[0].size();
        std::vector<float> flat_mat1(rows * cols), result_mat(rows * cols);
        for(int i=0; i<rows; ++i) memcpy(flat_mat1.data() + i*cols, mat1[i].data(), cols*sizeof(float));

        cl_mem bufA = ocl.createBuffer(sizeof(float) * rows * cols, CL_MEM_READ_WRITE);
        cl_mem bufC = ocl.createBuffer(sizeof(float) * rows * cols, CL_MEM_READ_WRITE);
        ocl.writeBuffer(bufA, sizeof(float) * rows * cols, flat_mat1.data());

        ocl.setKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
        ocl.setKernelArg(kernel, 1, sizeof(cl_mem), &bufC);
        ocl.setKernelArg(kernel, 2, sizeof(int), &rows);
        ocl.setKernelArg(kernel, 3, sizeof(int), &cols);

        size_t global_work_size[2] = {(size_t)cols, (size_t)rows};
        ocl.executeKernel(kernel, 2, global_work_size);
        ocl.readBuffer(bufC, sizeof(float) * rows * cols, result_mat.data());

        result_str = format_matrix(result_mat, cols, rows);

        ocl.releaseMemObject(bufA); ocl.releaseMemObject(bufC);
    } else if (operation == "matmul") {
        if (mat1.empty() || mat2.empty() || mat1[0].size() != mat2.size()) {
            return env->NewStringUTF("Invalid dimensions for matrix multiplication");
        }
        int M = mat1.size(); int N = mat1[0].size(); int K = mat2[0].size();
        std::vector<float> flat_mat1(M*N), flat_mat2(N*K), result_mat(M*K);
        for(int i=0; i<M; ++i) memcpy(flat_mat1.data() + i*N, mat1[i].data(), N*sizeof(float));
        for(int i=0; i<N; ++i) memcpy(flat_mat2.data() + i*K, mat2[i].data(), K*sizeof(float));

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

        size_t global_work_size[2] = {(size_t)M, (size_t)K};
        ocl.executeKernel(kernel, 2, global_work_size);
        ocl.readBuffer(bufC, sizeof(float) * M * K, result_mat.data());

        result_str = format_matrix(result_mat, M, K);

        ocl.releaseMemObject(bufA); ocl.releaseMemObject(bufB); ocl.releaseMemObject(bufC);
    }

    ocl.releaseKernel(kernel);
    ocl.releaseProgram(program);

    return env->NewStringUTF(result_str.c_str());
}
#endif

#if !defined(BUILD_ENGINE_CLI_DESKTOP)
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_openclandroidexample_MainActivity_dumpOpenCLDevices(
        JNIEnv* env, jobject) {
    std::string info = getOpenCLDevicesInfoString();
    return env->NewStringUTF(info.c_str());
}
#endif

#if !defined(BUILD_ENGINE_CLI_DESKTOP)
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_openclandroidexample_MainActivity_getGpuName(
        JNIEnv* env, jobject) {
    cl_platform_id platform_id = nullptr;
    cl_device_id device = nullptr;
    cl_int err = clGetPlatformIDs(1, &platform_id, nullptr);
    if (err != CL_SUCCESS) return env->NewStringUTF("");
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS) return env->NewStringUTF("");
    char deviceName[256] = {0};
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr);
    return env->NewStringUTF(deviceName);
}
#endif
