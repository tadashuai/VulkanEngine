#pragma once

#include "Core/Ref.h"

namespace VE
{

	class GraphicsContext : public ReferenceCount
	{
	public:
		GraphicsContext() = default;
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static Ref<GraphicsContext> Create();
	};

}
