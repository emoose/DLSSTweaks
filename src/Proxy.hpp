#pragma once

namespace proxy
{
extern bool is_wrapping_nvngx;
bool on_attach(HMODULE ourModule);
void on_detach();
};

namespace proxy_nvngx
{
bool on_attach(HMODULE ourModule);
void on_detach();
};

#define PLUGIN_API extern "C" __declspec(dllexport)
