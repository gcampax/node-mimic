//
// Copyright 2017 Giovanni Campagna <gcampagn@cs.stanford.edu>
//
// See COPYING for details

#include <algorithm>
#include <future>
#include <string>
#include <cstdint>

#include <uv.h>
#include <node/node.h>
#include <node/node_object_wrap.h>
#include <node/node_buffer.h>
#include <v8.h>

#include <mimic.h>

#include "lang_list.h"

using namespace v8;

namespace node_mimic {

static bool initialized = false;

static std::string
v8_to_string(const Local<v8::String>& s)
{
    int length = s->Length();
    char* buffer = new char[3*length];

    s->WriteUtf8(buffer, 3*length);
    std::string stdstring = std::string(buffer);
    delete[] buffer;
    return stdstring;
}

static Local<String> new_string(Isolate *isolate, const char *msg)
{
    return String::NewFromOneByte(isolate, (const uint8_t*)msg);
}

static void throw_exception(Isolate *isolate, const char *msg)
{
    isolate->ThrowException(Exception::TypeError(new_string(isolate, msg)));
}

template<class C>
class AutoObjectWrapRef;

template< class R >
class AsyncTask : private uv_work_t {
private:
    Isolate *isolate;
    std::function<R()> task;
    R result;
    std::function<Local<Value>(Isolate *isolate, const R&)> handle_result;
    Global<Function> callback;

    static void DoWork(uv_work_t* req) {
        AsyncTask *self = static_cast<AsyncTask*>(req);
        self->result = self->task();
    }

    static void CompleteWork(uv_work_t* req, int res) {
        AsyncTask *self = static_cast<AsyncTask*>(req);
        Isolate *isolate = self->isolate;
        R result = std::move(self->result);

        HandleScope scope(isolate);
        Local<Function> localCallback = self->callback.Get(isolate);
        Local<Value> jsResult = self->handle_result(isolate, result);
        Local<Value> argv[] = { Null(isolate), jsResult };
        node::MakeCallback(isolate, localCallback, localCallback, 2, argv);

        delete self;
    }

    template <class F, class G>
    explicit AsyncTask( Isolate *isolate_, F&& f, G&& g, Local<Function> callback_ ) :
        isolate(isolate_),
        task(std::forward<F>(f)),
        handle_result(std::forward<G>(g)),
        callback(isolate_, callback_) {}
public:
    template <class F, class G>
    static void Schedule( Isolate *isolate, F&& f, G&& g, Local<Function> callback_ ) {
        AsyncTask *self = new AsyncTask(isolate, std::forward<F>(f), std::forward<G>(g), callback_);
        uv_queue_work(uv_default_loop(), self, &DoWork, &CompleteWork);
    }
};

template<class C>
class AutoObjectWrapRef {
private:
    C* obj;

public:
    AutoObjectWrapRef(C& obj_) : obj(&obj_) {
        obj->Ref();
    }
    AutoObjectWrapRef(C* obj_) : obj(obj_) {
        obj->Ref();
    }

    AutoObjectWrapRef(const AutoObjectWrapRef& ref) : obj(ref.obj) {
        obj->Ref();
    }
    AutoObjectWrapRef(AutoObjectWrapRef&& ref) : obj(ref.obj) {
        ref.obj = nullptr;
    }
    ~AutoObjectWrapRef() {
        if (obj == nullptr)
            return;
        obj->Unref();
    }

    C& get() {
        return *obj;
    }
    const C& get() const {
        return *obj;
    }
};

static Local<Object> wave_to_js(Isolate *isolate, cst_wave *wave)
{
    Local<Object> obj = Object::New(isolate);
    obj->Set(new_string(isolate, "sampleRate"), Integer::New(isolate, wave->sample_rate));
    obj->Set(new_string(isolate, "numChannels"), Integer::New(isolate, wave->num_channels));

    size_t buffer_size = sizeof(int16_t) * wave->num_channels * wave->num_samples;
    obj->Set(new_string(isolate, "buffer"), node::Buffer::New(isolate, (char*)wave->samples, buffer_size, [](char *data, void *hint) {
        delete_wave((cst_wave*)hint);
    }, wave).ToLocalChecked());

    return obj;
}

class Voice : public node::ObjectWrap {
    friend class AutoObjectWrapRef<Voice>;

private:
    static Persistent<FunctionTemplate> constructor_template;
    static Persistent<Function> constructor;
    cst_voice *voice;

    Voice(cst_voice *voice_) : voice(voice_) {}
public:
    static Local<Function> Init(Isolate *isolate) {
        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, [](const FunctionCallbackInfo<Value>&){});
        tpl->SetClassName(String::NewFromUtf8(isolate, "Voice"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(tpl, "textToSpeech", TextToSpeech);

        constructor_template.Reset(isolate, tpl);
        Local<Function> constr = tpl->GetFunction();
        constructor.Reset(isolate, constr);
        return constr;
    }
    static Local<Object> NewInstance(Isolate *isolate, cst_voice *voice_) {
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        Local<Object> instance =
            cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr).ToLocalChecked();

        Voice *self = new Voice(voice_);
        self->Wrap(instance);
        return instance;
    }
    static Voice *Unwrap(Local<Object> obj) {
        return node::ObjectWrap::Unwrap<Voice>(obj);
    }
    static bool HasInstance(Local<Object> obj) {
        Isolate *isolate = obj->GetIsolate();
        return constructor_template.Get(isolate)->HasInstance(obj);
    }

    virtual ~Voice() {
        delete_voice(voice);
    }

    cst_voice *get() const {
        return voice;
    }

    static void TextToSpeech(const FunctionCallbackInfo<Value>& args) {
        Isolate *isolate = args.GetIsolate();

        if (args.Length() != 2)
            return throw_exception(isolate, "Must pass 2 arguments to textToSpeech()");

        Voice* self = Unwrap(args.Holder());

        MaybeLocal<String> text = args[0]->ToString(isolate->GetCurrentContext());
        if (text.IsEmpty())
            return;
        std::string cpp_text = v8_to_string(text.ToLocalChecked());
        if (!args[1]->IsFunction())
            return throw_exception(isolate, "2nd argument to textToSpeech() must be a function");

        if (!initialized)
            return throw_exception (isolate, "node-mimic not initialized");

        AutoObjectWrapRef<Voice> voice_ref(self);
        AsyncTask<cst_wave*>::Schedule(isolate, [voice_ref,cpp_text]() {
            return mimic_text_to_wave(cpp_text.c_str(), voice_ref.get().get());
        }, [](Isolate *isolate, cst_wave *wave) {
            if (wave == nullptr)
                return Null(isolate).As<Value>();
            return wave_to_js(isolate, wave).As<Value>();
        }, args[1].As<Function>());

        args.GetReturnValue().SetUndefined();
    }
};

Persistent<FunctionTemplate> Voice::constructor_template;
Persistent<Function> Voice::constructor;

static void Init(const FunctionCallbackInfo<Value>& args) {
    initialized = true;
    mimic_init();
    set_lang_list();

    node::AtExit([](void*) {
        mimic_exit();
    });

    args.GetReturnValue().SetUndefined();
}

static void LoadVoice(const FunctionCallbackInfo<Value>& args) {
    Isolate *isolate = args.GetIsolate();

    if (args.Length() != 2)
        return throw_exception(isolate, "Must pass 2 arguments to loadVoice()");

    MaybeLocal<String> voice_name = args[0]->ToString(isolate->GetCurrentContext());
    if (voice_name.IsEmpty())
        return;
    std::string cpp_voice_name = v8_to_string(voice_name.ToLocalChecked());
    if (!args[1]->IsFunction())
        return throw_exception(isolate, "2nd argument to loadVoice() must be a function");

    if (!initialized)
        return throw_exception (isolate, "node-mimic not initialized");

    AsyncTask<cst_voice*>::Schedule(isolate, [cpp_voice_name]() {
        return mimic_voice_load(cpp_voice_name.c_str());
    }, [](Isolate *isolate, cst_voice *voice) {
        if (voice == nullptr)
            return Null(isolate).As<Value>();
        return Voice::NewInstance(isolate, voice).As<Value>();
    }, args[1].As<Function>());

    args.GetReturnValue().SetUndefined();
}

void ModuleInit(Local<Object> exports) {
  NODE_SET_METHOD(exports, "init", Init);
  exports->Set(new_string(exports->GetIsolate(), "Voice"), Voice::Init(exports->GetIsolate()));
  NODE_SET_METHOD(exports, "loadVoice", LoadVoice);
}

}

NODE_MODULE(addon, node_mimic::ModuleInit)
