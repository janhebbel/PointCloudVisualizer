void CLGLUpdateSettings(open_cl *OpenCL, open_gl *OpenGL, uint32_t RenderWidth, uint32_t RenderHeight)
{
	cl_int Result;
	
	//
	// release the mem object from opencl, resize it in opengl, then recreate the opencl mem object
	clReleaseMemObject(OpenCL->Framebuffer);
    clReleaseMemObject(OpenCL->DepthBuffer);

    // destroy old gl texture
    glDeleteTextures(1, &OpenGL->framebuffer_texture);

    // recreate gl texture
    glGenTextures(1, &OpenGL->framebuffer_texture);
    OpenGL->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, OpenGL->framebuffer_texture);
    OpenGL->glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, RenderWidth, RenderHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // recreate cl mem object from gl texture
	OpenCL->Framebuffer = clCreateFromGLTexture(OpenCL->Context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, OpenGL->framebuffer_texture, &Result);
	assert(Result == CL_SUCCESS);

    // recreate cl mem object depth buffer
    OpenCL->DepthBuffer = clCreateBuffer(OpenCL->Context, CL_MEM_READ_WRITE, RenderWidth * RenderHeight * sizeof(unsigned int) * 2, NULL, &Result);
    assert(Result == CL_SUCCESS);
	
    OpenGL->framebuffer_width = RenderWidth;
    OpenGL->framebuffer_height = RenderHeight;
    OpenCL->FramebufferWidth = RenderWidth;
    OpenCL->FramebufferHeight = RenderHeight;
}
