#pragma once
#ifdef _AMD64_
extern "C" {
    void BreakInt3();
    void STDMETHODCALLTYPE _hookImplementFunction(void);
    int  STDMETHODCALLTYPE _callPreProcessing_x64(void* pnode, ULONG_PTR* __params, size_t params_size, void* pre_func);
    int  STDMETHODCALLTYPE _callOrginApiFunction_x64(ULONG_PTR* __params, size_t params_size, int __call_conv, void* __func, ULONG_PTR* __return);
}
#endif // #ifdef _AMD64_
