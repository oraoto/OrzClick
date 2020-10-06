#pragma once

#define DECLARE_METHOD(name) \
    Php::Value name(Php::Parameters &params)

#define DECLARE_VOID_METHOD(name) \
    void name(Php::Parameters &params)

#define REGISTER_METHOD(cls, type, name) \
    cls.method<&type::name>(#name);
