#include <node_buffer.h>
#include "spi.h"

void SPIInterface::Result(uv_work_t* req) {
  HandleScope scope;
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);

  Handle<Value> argv[2];
  if(baton->error_code) { // Error
    argv[0] = ;
    argv[1] = Undefined();
  } else {

  }

  // TODO Create Buffer

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

  if (!args[0]->IsFunction()) return ThrowException(Exception::TypeError(
    String::New("First argumernt must be a callback function")));
  Local<Function> callback = Local<Function>::Cast(args[0]);

  // Request Baton
  SPIBaton* baton = new SPIBaton();
  baton->error_code = 0;
  baton->callback = Persistent<Function>::New(callback);
  baton->payload = NULL;

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

NODE_MODULE(SPI, Init)
