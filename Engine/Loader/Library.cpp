namespace neo {

	/*
	void Library::imGuiEditor(ResourceManagers& resourceManagers) {
		TRACY_ZONE();

		ImGui::Begin("Library");

		// if (ImGui::TreeNodeEx("Framebuffers", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 	if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
		// 		ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
		// 		ImGui::TableSetupColumn("Attachments");
		// 		ImGui::TableHeadersRow();
		// 		for (auto& fbo : Library::mFramebuffers) {
		// 			ImGui::TableNextRow();
		// 			ImGui::TableSetColumnIndex(0);
		// 			ImGui::Text("%s", fbo.first.c_str());
		// 			ImGui::Text("[%d, %d]", fbo.second->mTextures[0]->mWidth, fbo.second->mTextures[0]->mHeight);
		// 			ImGui::TableSetColumnIndex(1);
		// 			for (auto t = fbo.second->mTextures.begin(); t < fbo.second->mTextures.end(); t++) {
		// 				textureFunc(**t);
		// 				if (t != std::prev(fbo.second->mTextures.end())) {
		// 					ImGui::SameLine();
		// 				}
		// 			}
		// 		}
		// 		for (auto& [details, tvList] : Library::mPooledFramebuffers) {
		// 			for (auto& tv : tvList) {
		// 				ImGui::TableNextRow();
		// 				ImGui::TableSetColumnIndex(0);
		// 				if (tv.mName) {
		// 					ImGui::Text("*%s", tv.mName.value().c_str());
		// 				}
		// 				ImGui::Text("[%d, %d]", tv.mFramebuffer->mTextures[0]->mWidth, tv.mFramebuffer->mTextures[0]->mHeight);
		// 				ImGui::TableSetColumnIndex(1);
		// 				for (auto t = tv.mFramebuffer->mTextures.begin(); t < tv.mFramebuffer->mTextures.end(); t++) {
		// 					textureFunc(**t);
		// 					if (t != std::prev(tv.mFramebuffer->mTextures.end())) {
		// 						ImGui::SameLine();
		// 					}
		// 				}
		// 			}
		// 		}
		// 		ImGui::EndTable();
		// 	}
		// 	ImGui::TreePop();
		// }
		// if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 	for (auto& t : Library::mTextures) {
		// 		if (ImGui::TreeNode(t.first.c_str())) {
		// 			ImGui::Text("[%d, %d]", t.second->mWidth, t.second->mHeight);
		// 			textureFunc(*t.second);
		// 			ImGui::TreePop();
		// 		}
		// 	}
		// 	ImGui::TreePop();
		// }
		ImGui::End();

	}
	*/
}
