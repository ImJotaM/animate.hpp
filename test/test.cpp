#include <raylib.h>

#define ANIMATE_HPP_IMPLEMENTATION
#include <animate.hpp>

int main() {
	
	InitWindow(800, 600, "Animation");
	SetTargetFPS(60);

	Rectangle rect = { 0.0f, 0.0f, 0.0f, 0.0f };
	Rectangle rect2 = { 0.0f, 0.0f, 0.0f, 0.0f };

	const AnimationId rect_to_screen_size = AnimationHandler::CreateAnimation({
		.onUpdate = [](float progress, void* obj){
			Rectangle* rect = static_cast<Rectangle*>(obj);

			float nw = rect->width;
			float nh = rect->height;
			
			nw = GetScreenWidth() * progress;
			nh = GetScreenHeight() * progress;
			
			rect->width = nw;
			rect->height = nh;
		},
		.onEachRepeatEnd = [](void* obj) {
			Rectangle* rect = static_cast<Rectangle*>(obj);
			rect->width = 0.0f;
			rect->height = 0.0f;
		}
	});

	AnimationHandler::AttachAnimation(rect_to_screen_size, &rect2, 3.0f, 2, {
		.onEnd = [rect_to_screen_size, &rect]() {
			AnimationHandler::AttachAnimation(rect_to_screen_size, &rect, 2.0f, 3, {
				.onEachRepeatEnd = [](void* obj){
					Rectangle* rect = static_cast<Rectangle*>(obj);
					rect->width = 0.0f;
					rect->height = 0.0f;
				}
			});
		}
	});

	while(!WindowShouldClose()) {
		
		float dt = GetFrameTime();

		AnimationHandler::UpdateAnimations(dt);

		BeginDrawing();
		ClearBackground(BLACK);

		DrawRectangleRec(rect, WHITE);
		DrawRectangleRec(rect2, RED);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}