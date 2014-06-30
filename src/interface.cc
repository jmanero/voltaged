#include "spi.h"

uv_work_t* SPIInterface::Request(Handle<Value> fd, uint8_t operation,
  Handle<Value> callback) {
  // Use the caller's HandleScope

  if (!fd->IsInteger()) return ThrowException(Exception::TypeError(
    String::New("File-handle must be an integer")));

  if (!callback->IsFunction()) return ThrowException(Exception::TypeError(
    String::New("callback must be a function")));

  // Request Baton
  SPIBaton* baton = new SPIBaton();
  baton->error_code = 0;
  baton->operation = operation;
  baton->fd = fd.Uint32Value();
  baton->callback = Persistent<Function>::Cast(callback);

  // UV Request
  uv_work_t* request = new uv_work_t();
  request->data = baton;

  return request
}

void SPIInterface::Result(uv_work_t* req) {
  HandleScope scope;
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  Handle<Value> argv[2];
  switch(baton->operation) {
    case SPI_INTERFACE_OPEN:
      argv[0] = Undefined();
      argv[1] = Number::New(baton->fd);
      break;
    case SPI_INTERFACE_TRANSFER:
      SPITransfer* transfer = (SPITransfer*)baton->payload;
      argv[0] = Undefined();
      argv[1] = baton->response;
      break;
    case SPI_INTERFACE_CLOSE:
      argv[0] = Undefined();
      argv[1] = Undefined();
      break;
    case SPI_INTERFACE_ERROR:
      argv[0] = Exception::Error(
        String::New("SPIDevice Error: " + baton->error_message +
        " (" + to_string(baton->error_code) + "): " +
        strerror(baton->error_code)));
      argv[1] = Undefined();
      break;
    default:
      argv[0] = Exception::TypeError(
        String::New("Unhandled SPIInterface operation " +
        to_string(baton->operation)));
      argv[1] = Undefined();
  }

  // Catch errors thrown from callback
  TryCatch try_catch;
  baton->callback->Call(Context::GetCurrent(), 2, argv);
  if (try_catch.HasCaught()) FatalException(try_catch);

  baton->callback.Dispose();

  // FREE THE MALLOCS!
  if(baton->payload) delete baton->payload;
  delete baton;
  delete req;
}

Handle<Value> SPIInterface::Open(const Arguments& args) {
  HandleScope scope;

  // Validate and cast arguments
  if (!args[0].isString()) return ThrowException(Exception::TypeError(
    String::New("Device must be a string")));
  String::Utf8Value device_(args[0]);

  if (!args[1].isNumber()) return ThrowException(Exception::TypeError(
    String::New("Mode must be a number")));
  uint8_t mode_ = (uint8_t) UInt32::Value(args[1]);

  if (!args[2].isNumber()) return ThrowException(Exception::TypeError(
    String::New("WordLength must be a number")));
  uint8_t word_ = (uint8_t) UInt32::Value(args[2]);

  if (!args[3].isNumber()) return ThrowException(Exception::TypeError(
    String::New("Speed must be a number")));
  uint32_t speed_ = UInt32::Value(args[3]);


  uv_work_t* request = Request(Number::New(-1), SPI_INTERFACE_TRANSFER, args[4]);
  request->data->device = device_;
  request->data->mode = mode_;
  request->data->word = word_;
  request->data->speed = speed_;

  uv_queue_work(uv_default_loop(), request, SPIDevice::open, SPIInterface::Result);
  return scope.Close(Undefined());
}

Handle<Value> SPIInterface::Transfer(const Arguments& args) {
  HandleScope scope;

  // Validate and cast arguments
  if (!args[1].isNumber()) return ThrowException(Exception::TypeError(
    String::New("WordLength must be a number")));
  uint8_t word_ = (uint8_t) UInt32::Value(args[1]);

  if (!args[2].isNumber()) return ThrowException(Exception::TypeError(
    String::New("Speed must be a number")));
  uint32_t speed_ = UInt32::Value(args[2]);

  if (!Buffer::HasInstance(args[3])) return ThrowException(Exception::TypeError(
    String::New("Send must be a Buffer")));
  Local<Object> send_ = args[3]->ToObject();

  if (!Buffer::HasInstance(args[4])) return ThrowException(Exception::TypeError(
    String::New("Receive must be a Buffer")));
  Local<Object>receive_ = args[4]->ToObject();

  uv_work_t* request = Request(args[0], SPI_INTERFACE_TRANSFER, args[5]);
  request->data->word = word_;
  request->data->speed = speed_;

  // Export read and write buffers
  request->data->send = &send_
  request->data->receive = &receive_;

  uv_queue_work(uv_default_loop(), request, SPIDevice::transfer, SPIInterface::Result);
  return scope.Close(Undefined());
}

Handle<Value> SPIInterface::Close(const Arguments& args) {
  HandleScope scope;
  uv_queue_work(uv_default_loop(),
    Request(args[0], SPI_INTERFACE_CLOSE, args[1]),
    SPIDevice::close, SPIInterface::Result);
  return scope.Close(Undefined());
}

void Init(Handle<Object> target) {
  HandleScope scope;

  // Export Methods
  target->Set(String::NewSymbol("open"),
    Persistent<Function>::New(SPIInterface::Open)->GetFunction());
  target->Set(String::NewSymbol("transfer"),
    Persistent<Function>::New(SPIInterface::Transfer)->GetFunction());
  target->Set(String::NewSymbol("close"),
    Persistent<Function>::New(SPIInterface::Close)->GetFunction());

  // Export constants
  Persistant<Object> constants = Object::New();
  target->Set(String::NewSymbol("CONST"), constants);

  constants->Set(String::NewSymbol("SPI_MODE_0"), Integer::New(SPI_MODE_0));
  constants->Set(String::NewSymbol("SPI_MODE_1"), Integer::New(SPI_MODE_1));
  constants->Set(String::NewSymbol("SPI_MODE_2"), Integer::New(SPI_MODE_2));
  constants->Set(String::NewSymbol("SPI_MODE_3"), Integer::New(SPI_MODE_3));
  constants->Set(String::NewSymbol("SPI_NO_CS"), Integer::New(SPI_NO_CS));
  constants->Set(String::NewSymbol("SPI_CS_HIGH"), Integer::New(SPI_CS_HIGH));
}

NODE_MODULE(spi_device, Init)
