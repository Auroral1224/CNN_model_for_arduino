#include <TensorFlowLite_ESP32.h>
#include <esp_task_wdt.h> 
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "baseline.h"


namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
constexpr int tensor_arena_size = 60*1024;
uint8_t tensor_arena[tensor_arena_size];
}

void setup() {
  // Define the input and output tensor shapes of the model
  const int INPUT_DIM = 28;
  const int OUTPUT_CLASSES = 10;
  const int INPUT_SIZE = INPUT_DIM * INPUT_DIM;
  const int TENSOR_INPUT[4] = {1, INPUT_DIM, INPUT_DIM, 1};
  const int TENSOR_OUTPUT[2] = {1, OUTPUT_CLASSES};

  Serial.begin(115200);
  
  // Set up logging
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Load the TFLite model from the memory buffer
  model = tflite::GetModel(baseline_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.\n",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  
  // Instantiate operations resolver
  static tflite::MicroMutableOpResolver<5> resolver;
  resolver.AddMaxPool2D();
  resolver.AddConv2D();
  resolver.AddFullyConnected();
  resolver.AddReshape();
  resolver.AddSoftmax();

  
  // Instantiate interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena,tensor_arena_size, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

    // Prepare the input tensor with test data
  float input_data[INPUT_SIZE] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0117645263671875, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.196044921875, 0.2783203125, 0.10589599609375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.60400390625, 0.84326171875, 0.533203125, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.560546875, 0.97265625, 0.58447265625, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0863037109375, 0.0863037109375, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.60009765625, 0.9609375, 0.470703125, 0.007843017578125, 0.007843017578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.07452392578125, 0.68603515625, 0.7216796875, 0.35693359375, 0.0, 0.0039215087890625, 0.0039215087890625, 0.007843017578125, 0.533203125, 0.96875, 0.474609375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.56884765625, 0.96875, 0.85888671875, 0.282470703125, 0.0, 0.01568603515625, 0.0117645263671875, 0.0, 0.56884765625, 0.98046875, 0.61181640625, 0.0863037109375, 0.039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.13330078125, 0.88623046875, 0.91748046875, 0.36474609375, 0.0, 0.0117645263671875, 0.0, 0.0, 0.199951171875, 0.85498046875, 0.984375, 0.87060546875, 0.87451171875, 0.411865234375, 0.0, 0.01568603515625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.341064453125, 0.953125, 0.6904296875, 0.023529052734375, 0.0, 0.0, 0.12548828125, 0.470703125, 0.7724609375, 1.0, 0.99609375, 0.85107421875, 0.91748046875, 0.454833984375, 0.0, 0.01568603515625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.478515625, 0.9765625, 0.63134765625, 0.054901123046875, 0.2744140625, 0.415771484375, 0.7216796875, 0.98828125, 0.96484375, 0.953125, 0.953125, 0.431396484375, 0.1529541015625, 0.07843017578125, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.537109375, 0.93310546875, 0.63525390625, 0.61962890625, 0.953125, 0.9765625, 0.93701171875, 0.7021484375, 0.36865234375, 0.7724609375, 0.89013671875, 0.2392578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.57666015625, 0.98046875, 0.9609375, 0.9609375, 0.81591796875, 0.56494140625, 0.290283203125, 0.07061767578125, 0.066650390625, 0.7919921875, 0.92919921875, 0.294189453125, 0.007843017578125, 0.01568603515625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.176513671875, 0.447021484375, 0.59619140625, 0.40380859375, 0.10589599609375, 0.0, 0.0, 0.0, 0.302001953125, 0.93310546875, 0.82763671875, 0.1607666015625, 0.0, 0.007843017578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.01568603515625, 0.007843017578125, 0.533203125, 0.953125, 0.521484375, 0.0117645263671875, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.01568603515625, 0.007843017578125, 0.0117645263671875, 0.007843017578125, 0.0039215087890625, 0.0, 0.04705810546875, 0.7373046875, 0.96484375, 0.443115234375, 0.0, 0.007843017578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.199951171875, 0.91748046875, 0.87451171875, 0.2078857421875, 0.0, 0.007843017578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.290283203125, 0.94921875, 0.65869140625, 0.04315185546875, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0, 0.419677734375, 0.96875, 0.7333984375, 0.035308837890625, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0039215087890625, 0.533203125, 0.95703125, 0.541015625, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.035308837890625, 0.6943359375, 0.953125, 0.38427734375, 0.0, 0.007843017578125, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0039215087890625, 0.0, 0.050994873046875, 0.7880859375, 0.99609375, 0.36865234375, 0.0, 0.0117645263671875, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0196075439453125, 0.176513671875, 0.2353515625, 0.0980224609375, 0.0, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.007843017578125, 0.0117645263671875, 0.0039215087890625, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  for (int i = 0; i < INPUT_SIZE; i++) {
    input->data.f[i] = input_data[i]; // The input data is between 0 and 255
  }

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed.\n");
    return;
  }

  // Print the output tensor values
  for (int i = 0; i < OUTPUT_CLASSES; i++) {
    Serial.print(output->data.f[i]);
    Serial.print("\t");
  }
  Serial.println();

  // Calculate the predicted class
  int predicted_class = 0;
  float max_value = output->data.f[0];
  for (int i = 1; i < OUTPUT_CLASSES; i++) {
    if (output->data.f[i] > max_value) {
      max_value = output->data.f[i];
      predicted_class = i;
    }
  }

  // Print the predicted class
  Serial.print("Predicted class: ");
  Serial.println(predicted_class);
}

void loop() {
}
