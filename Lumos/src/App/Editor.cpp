#include "LM.h"
#include "Editor.h"
#include "App/Application.h"
#include <imgui/plugins/ImGuizmo.h>
#include "Entity/Entity.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Camera/Camera.h"
#include "App/Engine.h"
#include "Physics/LumosPhysicsEngine/LumosPhysicsEngine.h"
#include "App/SceneManager.h"
#include "App/Scene.h"
#include "Graphics/RenderManager.h"
#include "Graphics/API/Textures/Texture2D.h"
#include "Console.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "Maths/BoundingSphere.h"

namespace Lumos
{
	Editor::Editor(Application* app, uint width, uint height) : m_Application(app)
	{
		m_Console = new Console();
		LMLog::GetCoreLogger()->sinks().emplace_back(std::make_shared<ConsoleSink>(*m_Console));
		LMLog::GetClientLogger()->sinks().emplace_back(std::make_shared<ConsoleSink>(*m_Console));

		m_SceneViewSize = maths::Vector2(static_cast<float>(width), static_cast<float>(height));
	}


	Editor::~Editor()
	{
		LMLog::GetCoreLogger()->sinks().pop_back();
		LMLog::GetClientLogger()->sinks().pop_back();
		delete m_Console;

		for (auto texture : m_Icons)
			delete texture.second;
	}

	void Editor::OnImGui()
	{
		DrawMenuBar();
		BeginDockSpace();
		//SelectEntity();
		DrawSceneView();
		DrawConsole();
		DrawHierarchyWindow();
		DrawInspectorWindow();
		EndDockSpace();
	}

	void Editor::DrawMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) { Application::Instance()->SetAppState(AppState::Closing); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::MenuItem("Console")) {}
				if (ImGui::MenuItem("Hierarchy", "", true, true)) {}
				if (ImGui::MenuItem("Scene", "")) {}
				if (ImGui::MenuItem("Inspector", "")) {}
				ImGui::EndMenu();
			}

			ImGui::SameLine(ImGui::GetWindowContentRegionMax().x / 2.0f);
			if (ImGui::ImageButton(m_Icons["play"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_Application->SetEditorState(EditorState::Play);
			
			if (ImGui::ImageButton(m_Icons["pause"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_Application->SetEditorState(EditorState::Paused);

			if (ImGui::ImageButton(m_Icons["next"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_Application->SetEditorState(EditorState::Next);

			ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 330.0f);
			ImGui::Text("Application average %.3f ms/frame (%i FPS)", 1000.0f / (float)Engine::Instance()->GetFPS(), Engine::Instance()->GetFPS());

			ImGui::EndMainMenuBar();
		}
	}

	void Editor::DrawHierarchyWindow()
	{
		ImGui::Begin("Hierarchy", nullptr, 0);
		{
			if (ImGui::TreeNode("Application"))
			{
				/*		if (ImGui::RadioButton("Translate", m_ImGuizmoOperation == ImGuizmo::TRANSLATE))
					m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
				if (ImGui::RadioButton("Rotate", m_ImGuizmoOperation == ImGuizmo::ROTATE))
					m_ImGuizmoOperation = ImGuizmo::ROTATE;
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale", m_ImGuizmoOperation == ImGuizmo::SCALE))
					m_ImGuizmoOperation = ImGuizmo::SCALE;*/

				if (ImGui::ImageButton(m_Icons["translate"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
					m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
				if (ImGui::ImageButton(m_Icons["rotate"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
					m_ImGuizmoOperation = ImGuizmo::ROTATE;
				ImGui::SameLine();
				if (ImGui::ImageButton(m_Icons["scale"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
					m_ImGuizmoOperation = ImGuizmo::SCALE;

				ImGui::NewLine();
				LumosPhysicsEngine::Instance()->OnImGUI();
				ImGui::NewLine();
				ImGui::Text("FPS : %5.2i", Engine::Instance()->GetFPS());
				ImGui::Text("UPS : %5.2i", Engine::Instance()->GetUPS());
				ImGui::Text("Frame Time : %5.2f ms", Engine::Instance()->GetFrametime());
				ImGui::NewLine();
				ImGui::Text("Scene : %s", m_Application->m_SceneManager->GetCurrentScene()->GetSceneName().c_str());

				if (ImGui::TreeNode("GBuffer"))
				{
					if (ImGui::TreeNode("Colour Texture"))
					{
						ImGui::Image(m_Application->m_RenderManager->GetGBuffer()->m_ScreenTex[SCREENTEX_COLOUR]->GetHandle(), ImVec2(128, 128), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f));
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("Normal Texture"))
					{
						ImGui::Image(m_Application->m_RenderManager->GetGBuffer()->m_ScreenTex[SCREENTEX_NORMALS]->GetHandle(), ImVec2(128, 128), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f));
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("PBR Texture"))
					{
						ImGui::Image(m_Application->m_RenderManager->GetGBuffer()->m_ScreenTex[SCREENTEX_PBR]->GetHandle(), ImVec2(128, 128), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f));
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("Position Texture"))
					{
						ImGui::Image(m_Application->m_RenderManager->GetGBuffer()->m_ScreenTex[SCREENTEX_POSITION]->GetHandle(), ImVec2(128, 128), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f));
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			
			m_Application->m_SceneManager->GetCurrentScene()->OnIMGUI();
			if (ImGui::TreeNode("Scene"))
			{
				auto scene = m_Application->m_SceneManager->GetCurrentScene();

				if(scene->GetLightSetup())
					scene->GetLightSetup()->OnImGUI();

				auto entities = scene->GetEntities();
				scene->GetCamera()->OnImGUI();
				
				std::string title = "Entities : " + StringFormat::ToString(static_cast<int>(entities.size()));

				if (ImGui::TreeNode(title.c_str()))
				{
					for (auto& entity : entities)
					{
						if (ImGui::Selectable(entity->GetName().c_str(), m_Selected == entity.get()))
							m_Selected = entity.get();
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	void Editor::DrawEngineInfoWindow()
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin("Engine", NULL, window_flags);
		{
			if (ImGui::ImageButton(m_Icons["translate"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if (ImGui::ImageButton(m_Icons["rotate"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_ImGuizmoOperation = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if (ImGui::ImageButton(m_Icons["scale"]->GetHandle(), ImVec2(16, 16), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f)))
				m_ImGuizmoOperation = ImGuizmo::SCALE;

			ImGui::NewLine();
			ImGui::Text("FPS : %5.2i", Engine::Instance()->GetFPS());
			ImGui::Text("UPS : %5.2i", Engine::Instance()->GetUPS());
			ImGui::Text("Frame Time : %5.2f ms", Engine::Instance()->GetFrametime());
			ImGui::NewLine();
			ImGui::Text("Scene : %s", m_Application->m_SceneManager->GetCurrentScene()->GetSceneName().c_str());
		}
		ImGui::End();
	}

	void Editor::DrawInspectorWindow()
	{
		ImGui::Begin("Inspector", NULL, 0);
		if (m_Selected)
			m_Selected->OnIMGUI();
		ImGui::End();
	}

	void Editor::DrawSceneView()
	{
		ImGuiWindowFlags windowFlags = 0;
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Scene", nullptr, windowFlags);
		
		ImGuizmo::SetDrawlist();
		m_SceneViewSize = maths::Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
		maths::Vector2 m_SceneViewPosition = maths::Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

		m_Application->m_SceneManager->GetCurrentScene()->GetCamera()->SetAspectRatio(static_cast<float>(ImGui::GetWindowSize().x) / static_cast<float>(ImGui::GetWindowSize().y));

		auto width = static_cast<unsigned int>(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x);
		auto height = static_cast<unsigned int>(ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);

		// Make pixel perfect
		width -= (width % 2 != 0) ? 1 : 0;
		height -= (height % 2 != 0) ? 1 : 0;

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, static_cast<float>(width), static_cast<float>(height));
		ImGui::Image(m_Application->m_RenderManager->GetGBuffer()->m_ScreenTex[SCREENTEX_OFFSCREEN0]->GetHandle(), ImVec2(static_cast<float>(width), static_cast<float>(height)), ImVec2(0.0f, m_FlipImGuiImage ? 1.0f : 0.0f), ImVec2(1.0f, m_FlipImGuiImage ? 0.0f : 1.0f));

		if (m_Selected)
		{
			m_Selected->OnGuizmo(m_ImGuizmoOperation);
		}

		ImGui::End();
	}

	void Editor::BeginDockSpace()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		auto windowFlags
			= ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_MenuBar
			| ImGuiDockNodeFlags_PassthruDockspace;

		ImGui::Begin("DockSpace", nullptr, windowFlags);

		ImGui::PopStyleVar(3);

		if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) == NULL)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id);

			ImGuiID dock_main_id = dockspace_id; 
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_main_id);

			ImGui::DockBuilderDockWindow("Scene", dock_id_right);
			ImGui::DockBuilderDockWindow("Inspector", dock_id_left);
			ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
			ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
			ImGui::DockBuilderDockWindow("Engine", dock_id_left);
			ImGui::DockBuilderDockWindow("Scene Information", dock_id_left);
			ImGui::DockBuilderDockWindow("ImGui Demo", dock_id_left);

			ImGui::DockBuilderFinish(dockspace_id);
		}
		

		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruDockspace;
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	void Editor::EndDockSpace()
	{
		ImGui::End();
	}

	void Editor::SelectEntity()
	{
		bool bCheck = false;
		bCheck |= !ImGui::IsMouseClicked(0);

		bCheck |= !ImGui::IsWindowHovered
		(
			ImGuiHoveredFlags_AllowWhenBlockedByPopup |
			ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
		);

		if (bCheck)
			return;

		maths::Vector2 relativeMousePos = Input::GetInput().GetMousePosition() - m_SceneViewPosition;

		float pointX = +2.0f * relativeMousePos.x / m_SceneViewSize.x - 1.0f;
		float pointY = -2.0f * relativeMousePos.y / m_SceneViewSize.y + 1.0f;

		maths::Matrix4 view = m_Application->GetSceneManager()->GetCurrentScene()->GetCamera()->GetViewMatrix();
		maths::Matrix4 proj = m_Application->GetSceneManager()->GetCurrentScene()->GetCamera()->GetProjectionMatrix();
		maths::Matrix4 invProjView = maths::Matrix4::Inverse(view * proj);

		maths::Vector3 worldMousePos = invProjView * maths::Vector3(pointX, pointY, 0.0f);

		auto entities = m_Application->m_SceneManager->GetCurrentScene()->GetEntities();

		for (auto& entity : entities)
		{
			auto boundingBox = entity->GetBoundingRadius();
			maths::BoundingSphere test(entity->GetTransform()->m_Transform.GetWorldPosition(), boundingBox);
			if (test.Intersects(worldMousePos))
			{
				m_Selected = entity.get();
				return;
			}
		}
	}

	void Editor::OnInit()
	{
		//Load Icons
		m_Icons["play"] = Texture2D::CreateFromFile("playIcon", "/CoreTextures/Editor/icons/play.png");
		m_Icons["pause"] = Texture2D::CreateFromFile("pauseIcon", "/CoreTextures/Editor/icons/pause.png");
		m_Icons["next"] = Texture2D::CreateFromFile("nextIcon", "/CoreTextures/Editor/icons/next.png");

		m_Icons["translate"] = Texture2D::CreateFromFile("translateIcon", "/CoreTextures/Editor/icons/translate.png");
		m_Icons["scale"] = Texture2D::CreateFromFile("scaleIcon", "/CoreTextures/Editor/icons/scale.png");
		m_Icons["rotate"] = Texture2D::CreateFromFile("rotateIcon", "/CoreTextures/Editor/icons/rotate.png");


	}

	void Editor::OnNewScene(Scene * scene)
	{
		m_Selected = nullptr;
	}

	void Editor::DrawConsole()
	{
		m_Console->Draw("Console");
	}
}
