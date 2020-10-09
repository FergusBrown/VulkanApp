#include "Subpass.h"

#include "RenderTarget.h"

Subpass::Subpass(std::string vertexShaderSource, std::string fragmentShaderSource) :
	mVertexShaderSource(vertexShaderSource), mFragmentShaderSource(fragmentShaderSource)
{
}

const std::string& Subpass::vertexShaderSource() const
{
	return mVertexShaderSource;
}

const std::string& Subpass::fragmentShaderSource() const
{
	return mFragmentShaderSource;
}

const std::vector<uint32_t> Subpass::inputAttachments() const
{
	return mInputAttachments;
}

const std::vector<uint32_t> Subpass::outputAttachments() const
{
	return mOutputAttachments;
}

void Subpass::setInputAttachments(const std::vector<uint32_t>& inputAttachments)
{
	mInputAttachments = inputAttachments;
}

void Subpass::setOutputAttachments(const std::vector<uint32_t>& outputAttachments)
{
	mOutputAttachments = outputAttachments;
}

void Subpass::updateRenderTargetAttachments(RenderTarget& renderTarget)
{
	renderTarget.setInputAttachments(mInputAttachments);
	renderTarget.setOutputAttachments(mOutputAttachments);
}
