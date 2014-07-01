#include "spi.h"

/**
 * Build a new uv_work_t with an SPIBaton
 */
uv_work_t* SPIInterface::create_request(Local<Value> fd, uint8_t operation, Local<Value> callback) {
  SPIBaton* baton = new SPIBaton(); // Create Request Baton
  baton->error_code = 0;
  baton->operation = operation;
  baton->fd = fd->ToInt32()->Value();
  baton->callback = Persistent<Function>::New(Local<Function>::Cast(callback));

  uv_work_t* request = new uv_work_t(); // Create UV Request
  request->data = baton;

  return request;
}

/**
 * Validate `fd` and `callback` before trying to build a request
 */
Handle<Value> SPIInterface::validate_request(Local<Value> fd, Local<Value> callback) {
  if (!fd->IsInt32()) return ThrowException(Exception::TypeError(
    String::New("File-handle must be an integer")));

  if (!callback->IsFunction()) return ThrowException(Exception::TypeError(
    String::New("callback must be a function")));

  return Undefined();
}

/**
 * Handle callbacks from SPIDevice tasks
 */
void SPIInterface::Result(uv_work_t* req) {
  HandleScope scope;
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  Handle<Value> argv[] = { Undefined(), Undefined() };
  TryCatch try_catch;
  switch(baton->operation) {
    case SPI_INTERFACE_OPEN:
      argv[1] = Int32::New(baton->fd);
      delete baton->device; // FREE THE MALLOCS!!
      break;

    case SPI_INTERFACE_TRANSFER:
    case SPI_INTERFACE_CLOSE:
      break;

    case SPI_INTERFACE_ERROR:
      argv[0] = Exception::Error(
        String::New(("SPIDevice Error: " + baton->error_message +
        " (" + to_string(baton->error_code) + "): " +
        strerror(baton->error_code)).c_str()));
      break;

    default:
      argv[0] = Exception::TypeError(
        String::New(("Unhandled SPIInterface operation " +
        to_string(baton->operation)).c_str()));
  }
  baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  if (try_catch.HasCaught()) FatalException(try_catch);

  // FREE THE MALLOCS!!
  baton->callback.Dispose();
  delete baton;
  delete req;
}

/**
 * Enqueue an open task
 */
Handle<Value> SPIInterface::Open(const Arguments& args) {
  HandleScope scope;

  // Validate and cast arguments
  if (!args[0]->IsString()) return ThrowException(Exception::TypeError(
    String::New("Device must be a string")));
  String::Utf8Value device_(args[0]->ToString());

  if (!args[1]->IsNumber()) return ThrowException(Exception::TypeError(
    String::New("Mode must be a number")));
  uint8_t mode_ = (uint8_t) args[1]->ToUint32()->Value();

  if (!args[2]->IsNumber()) return ThrowException(Exception::TypeError(
    String::New("WordLength must be a number")));
  uint8_t word_ = (uint8_t) args[2]->ToUint32()->Value();

  if (!args[3]->IsNumber()) return ThrowException(Exception::TypeError(
    String::New("Speed must be a number")));
  uint32_t speed_ = args[3]->ToUint32()->Value();

  Local<Number> fd = Int32::New(-1);
  validate_request(fd, args[4]);
  uv_work_t* request = create_request(fd, SPI_INTERFACE_OPEN, args[4]);
  SPIBaton* baton = (SPIBaton*)(request->data);

  // <sad-hack> Will be freed in Response.
  baton->device = (char*)calloc(device_.length(), sizeof(char));
  memcpy(baton->device, *device_, device_.length());
  // </sad-hack>

  baton->mode = mode_;
  baton->word = word_;
  baton->speed = speed_;

  uv_queue_work(uv_default_loop(), request, SPIDevice::open,
    (uv_after_work_cb)(SPIInterface::Result));
  return scope.Close(Undefined());
}

/**
 * Enqueue a transfer task
 */
Handle<Value> SPIInterface::Transfer(const Arguments& args) {
  HandleScope scope;

  // Validate and cast arguments
  if (!args[1]->IsNumber()) return ThrowException(Exception::TypeError(
    String::New("WordLength must be a number")));
  uint8_t word_ = (uint8_t) args[1]->ToUint32()->Value();

  if (!args[2]->IsNumber()) return ThrowException(Exception::TypeError(
    String::New("Speed must be a number")));
  uint32_t speed_ = args[2]->ToUint32()->Value();

  if (!Buffer::HasInstance(args[3])) return ThrowException(Exception::TypeError(
    String::New("Send must be a Buffer")));
  Local<Object> send_ = args[3]->ToObject();

  if (!Buffer::HasInstance(args[4])) return ThrowException(Exception::TypeError(
    String::New("Receive must be a Buffer")));
  Local<Object>receive_ = args[4]->ToObject();

  validate_request(args[0], args[5]);
  uv_work_t* request = create_request(args[0], SPI_INTERFACE_TRANSFER, args[5]);
  SPIBaton* baton = (SPIBaton*)(request->data);
  baton->word = word_;
  baton->speed = speed_;

  // Export read and write buffers
  baton->length = Buffer::Length(send_);
  baton->send = (uint8_t*)(Buffer::Data(send_));
  baton->receive = (uint8_t*)(Buffer::Data(receive_));

  uv_queue_work(uv_default_loop(), request, SPIDevice::transfer,
    (uv_after_work_cb)(SPIInterface::Result));
  return scope.Close(Undefined());
}

/**
 * Enqueue a close task
 */
Handle<Value> SPIInterface::Close(const Arguments& args) {
  HandleScope scope;

  validate_request(args[0], args[1]);
  uv_queue_work(uv_default_loop(),
    create_request(args[0], SPI_INTERFACE_CLOSE, args[1]),
    SPIDevice::close, (uv_after_work_cb)(SPIInterface::Result));
  return scope.Close(Undefined());
}

/**
 * Ye Old V8 Init handler
 */
void Init(Handle<Object> target) {
  HandleScope scope;

  // Export Methods
  target->Set(String::NewSymbol("open"),
    FunctionTemplate::New(SPIInterface::Open)->GetFunction());
  target->Set(String::NewSymbol("transfer"),
    FunctionTemplate::New(SPIInterface::Transfer)->GetFunction());
  target->Set(String::NewSymbol("close"),
    FunctionTemplate::New(SPIInterface::Close)->GetFunction());

  // Export constants
  Persistent<Object> constants = Persistent<Object>::New(Object::New());
  target->Set(String::NewSymbol("CONST"), constants);

  constants->Set(String::NewSymbol("SPI_MODE_0"), Integer::New(SPI_MODE_0));
  constants->Set(String::NewSymbol("SPI_MODE_1"), Integer::New(SPI_MODE_1));
  constants->Set(String::NewSymbol("SPI_MODE_2"), Integer::New(SPI_MODE_2));
  constants->Set(String::NewSymbol("SPI_MODE_3"), Integer::New(SPI_MODE_3));
  constants->Set(String::NewSymbol("SPI_NO_CS"), Integer::New(SPI_NO_CS));
  constants->Set(String::NewSymbol("SPI_CS_HIGH"), Integer::New(SPI_CS_HIGH));
}

/**
 * Node JS Black Magic (tm)
 */
NODE_MODULE(SPIInterface, Init);
