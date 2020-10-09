#pragma once
#include "Common.h"

class RenderTarget;

// Object which stores the indices of attachments used for a particular subpass
class Subpass
{
public:
	Subpass(std::string vertexShaderSource, std::string fragmentShaderSource);
	~Subpass() = default;

	Subpass(const Subpass&) = delete;

	// - Getters
	const std::string& vertexShaderSource() const;
	const std::string& fragmentShaderSource() const;
	const std::vector<uint32_t> inputAttachments() const;
	const std::vector<uint32_t> outputAttachments() const;

	// - Setters
	void setInputAttachments(const std::vector<uint32_t>& inputAttachments = {});
	void setOutputAttachments(const std::vector<uint32_t>& outputAttachments = {});

	// Update render target with this subpass's attachment indices
	void updateRenderTargetAttachments(RenderTarget& renderTarget);

private:

	std::string mVertexShaderSource{};
	std::string mFragmentShaderSource{};

	// No input attachments by default
	std::vector<uint32_t> mInputAttachments = {};

	// Default output attachment is swapchain
	std::vector<uint32_t> mOutputAttachments = { 0 };
};

