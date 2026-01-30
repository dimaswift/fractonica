#import "ViewController.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include "imgui.h"
#include "backends/imgui_impl_metal.h"
#include <LunarTime.h>
#include <iostream>
#include <chrono>
#include <ctime>

@interface ViewController () <MTKViewDelegate>
@end

@implementation ViewController {
    id<MTLDevice> _device;
    id<MTLCommandQueue> _queue;
    MTKView *_view;
    Fractonica::LunarTime time;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    _device = MTLCreateSystemDefaultDevice();
    _queue = [_device newCommandQueue];

    _view = [[MTKView alloc] initWithFrame:self.view.bounds device:_device];
    _view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _view.delegate = self;
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
    [self.view addSubview:_view];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplMetal_Init(_device);
}

long long getCurrentUnixTimestamp() {

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration = now.time_since_epoch();
    long long unix_timestamp = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return unix_timestamp;
}


- (void)dealloc {
    ImGui_ImplMetal_Shutdown();
    ImGui::DestroyContext();
}

#pragma mark - MTKViewDelegate

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    (void)view; (void)size;
}

void renderPeriod(const Fractonica::LunarEventInfo info) {
    
    ImGui::Text("Bin: %d", info.binOctal);
    ImGui::Text("Normalized: %f", info.normalized);
    ImGui::Text("Progress: %f", info.progress);
}

- (void)drawInMTKView:(MTKView *)view {
    id<CAMetalDrawable> drawable = view.currentDrawable;
       MTLRenderPassDescriptor *rp = view.currentRenderPassDescriptor;
       if (!drawable || !rp) return;


       ImGuiIO& io = ImGui::GetIO();
       CGSize size = view.bounds.size;
       io.DisplaySize = ImVec2((float)size.width, (float)size.height);

       CGFloat scale = 1.0;
       if (view.window.screen) scale = view.window.screen.scale;
       else scale = UIScreen.mainScreen.scale;
       io.DisplayFramebufferScale = ImVec2((float)scale, (float)scale);
       // ---------------------------------------------------------

       id<MTLCommandBuffer> cb = [_queue commandBuffer];

       ImGui_ImplMetal_NewFrame(rp);
       ImGui::NewFrame();

    auto now = static_cast<uint32_t>(getCurrentUnixTimestamp());
    
    auto newMoon = time.getEventInfo(now, Fractonica::NEW_MOON);
    auto apogee = time.getEventInfo(now, Fractonica::APOGEE);
    auto nodal = time.getEventInfo(now, Fractonica::NODAL_ASCENDING);
    
    ImGui::Begin("Periods");
    ImGui::Separator();
    ImGui::Text("New moon:");
    renderPeriod(newMoon);
    ImGui::Separator();
    ImGui::Text("Apogee:");
    renderPeriod(apogee);
    ImGui::Separator();
    ImGui::Text("Node:");
    renderPeriod(nodal);
    
    ImGui::End();

    ImGui::Render();

       id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rp];
       ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, enc);
       [enc endEncoding];

       [cb presentDrawable:drawable];
       [cb commit];
}

@end
