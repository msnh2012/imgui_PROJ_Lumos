#pragma once
#include "LM.h"
#include "LumosComponent.h"

namespace Lumos
{
	class SoundNode;

	class LUMOS_EXPORT SoundComponent : public LumosComponent
	{
	public:
        SoundComponent();
		explicit SoundComponent(std::shared_ptr<SoundNode>& sound);

		void OnUpdateComponent(float dt) override;
		void DebugDraw(uint64 debugFlags) override;
		void Init() override;

		void OnIMGUI() override;
        
        SoundNode* GetSoundNode() const { return m_SoundNode.get(); }
        
    private:
        std::shared_ptr<SoundNode> m_SoundNode;
	};
}
