#include <node_buffer.h>
#include "spi.h"

uv_work_t SPIInterface::Request(uint32_t fd, uint8_t operation,
  void* payload, Persistent<Function> callback) {
  HandleScope scope;

  // Request Baton
  SPIBaton* baton = new SPIBaton();
  baton->error_code = 0;
  baton->operation = operation;
  baton->payload = payload;
  baton->callback = callback;

  // UV Request
  uv_work_t request = new uv_work_t();
  request.data = baton;

  return request
}

void SPIInterface::Result(uv_work_t* req) {
  HandleScope scope;
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  Handle<Value> argv[2];
  switch(baton->operation) {
    case SPI_INTERFACE_OPEN:
      argv[0] = Undefined();
      argv[1] = Number(baton->fd); // TODO
      break;
    case SPI_INTERFACE_TRANSFER:
      argv[0] = Undefined();
      argv[1] = Buffer(data); // TODO
      break;
    case SPI_INTERFACE_CLOSE:
      argv[0] = Undefined();
      argv[1] = Undefined();
      break;
    case SPI_INTERFACE_ERROR:
      argv[0] = Error; // TODO
      argv[1] = Undefined();
      break;
    default:
      argv[0] = Error; // TODO
      argv[1] = Undefined();
  }

  TryCatch try_catch;
  baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  if (try_catch.HasCaught()) FatalException(try_catch);

  baton->callback.Dispose();

  // FREE THE MALLOCS!
  if(baton->payload) delete baton->payload;
  delete baton;
  delete req;
}

Handle<Value> SPIInterface::Open(const Arguments& args) {
  HandleScope scope;

  // TODO Argument Validation

  SPISetup* payload = new SPISetup();

  uv_work_t request = Request(-1, SPI_INTERFACE_OPEN, payload, )
  uv_queue_work(uv_default_loop(), &request, SPIDevice::open, SPIInterface::Result);
  return scope.Close(Undefined());
}

Handle<Value> SPIInterface::Transfer(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsInteger()) return ThrowException(Exception::TypeError(
    String::New("First argumernt must be an Integer filehandle")));

  if (!Buffer::HasInstance(args[1])) return ThrowException(Exception::TypeError(
    String::New("Second argumernt must be a Buffer")));
  Local<Object> data = args[1]->ToObject()

  if (!args[2]->IsFunction()) return ThrowException(Exception::TypeError(
    String::New("Third argumernt must be a callback function")));
  Local<Function> callback = Local<Function>::Cast(args[2]);

  // Request Baton
  SPIBaton* baton = new SPIBaton();
  baton->error_code = 0;
  baton->callback = Persistent<Function>::New(callback);
  baton->operation = SPI_INTERFACE_TRANSFER;
  baton->fd = args[0].Uint32Value();

  // Transfer Payload
  baton->payload = new SPITransfer();
  baton->payload->data = Buffer::Data(data);
  baton->payload->length = Buffer::Length(data);

  // UV Request
  uv_work_t request = new uv_work_t();
  request.data = baton;

  uv_queue_work(uv_default_loop(), &request, SPIDevice::transfer, SPIInterface::Result);
  return scope.Close(Undefined());
}

Handle<Value> SPIInterface::Close(const Arguments& args) {
  HandleScope scope;

  if (!args[1]->IsFunction()) return ThrowException(Exception::TypeError(
    String::New("Second argumernt must be a callback function")));
  Local<Function> callback = Local<Function>::Cast(args[1]);

  // Request Baton
  SPIBaton* baton = new SPIBaton();
  baton->error_code = 0;
  baton->callback = Persistent<Function>::New(callback);
  baton->operation = SPI_INTERFACE_CLOSE;
  baton->fd = args[0].Uint32Value();
  baton->payload = NULL;

  // UV Request
  uv_work_t request = new uv_work_t();
  request.data = baton;

  uv_queue_work(uv_default_loop(), &request, SPIDevice::transfer, SPIInterface::Result);
  return scope.Close(Undefined());
}

void Init(Handle<Object> target) {
  HandleScope scope;

  target->Set(String::NewSymbol("open"),
    Persistent<Function>::New(SPIInterface::Open)->GetFunction());
  target->Set(String::NewSymbol("transfer"),
    Persistent<Function>::New(SPIInterface::Transfer)->GetFunction());
  target->Set(String::NewSymbol("close"),
    Persistent<Function>::New(SPIInterface::Close)->GetFunction());

  scope.Close();
}

NODE_MODULE(spi_device, Init)
