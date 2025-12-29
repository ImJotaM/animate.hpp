#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>

#ifdef ANIM_NAMESPACE
namespace Anim {
#endif

typedef size_t AnimationId;
typedef size_t InstanceId;

typedef std::function<void()> AnimationOnStartFunction;
typedef std::function<void(void*)> AnimationOnEachRepeatStart;
typedef std::function<void(float, void*)> AnimationUpdateFunction;
typedef std::function<void(void*)> AnimationOnEachRepeatEnd;
typedef std::function<void()> AnimationOnEnd;

struct AnimationEvents {
	std::function<void()> onStart = nullptr;
	std::function<void(void*)> onEachRepeatStart = nullptr;
	AnimationUpdateFunction onUpdate = nullptr;
	std::function<void(void*)> onEachRepeatEnd = nullptr;
	std::function<void()> onEnd = nullptr;
};

enum AnimationState {
	ANIM_STARTING = 0,
	ANIM_RUNNING,
	ANIM_PAUSED,
	ANIM_STOPPING,
	ANIM_FINISHED,
};

struct AnimationInstance {

	InstanceId id = 0;

	void* obj = nullptr;
	AnimationEvents events = { };

	enum AnimationState state = ANIM_STARTING;

	float duration = 0.0f;
	size_t repeat = 0;
	
	float time = 0.0f;
	size_t repeat_count = 0;
};

class Animation {

public:

	Animation(AnimationEvents events);
	~Animation() = default;

	AnimationEvents events = { };

};

class AnimationHandler {

public:
	static const AnimationId CreateAnimation(AnimationEvents events);
	static InstanceId AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, AnimationEvents events);
	static void UpdateAnimations(float dt);

	static bool HasAnimation(AnimationId id);
	static void RemoveAnimation(AnimationId id);
	static void ClearAnimations();

	static void Pause(InstanceId i_id);
	static void Stop(InstanceId id);
	static void Continue(InstanceId id);
	static void Restart(InstanceId id);

private:

	static size_t s_animation_count;
	static std::unordered_map<AnimationId, std::unique_ptr<Animation>> s_animations;

	static size_t s_instace_count;
	static std::vector<AnimationInstance> s_instances;

};

#ifdef ANIM_NAMESPACE
}
#endif

#ifdef ANIMATE_HPP_IMPLEMENTATION

#ifdef ANIM_NAMESPACE
namespace Anim {
#endif

Animation::Animation(AnimationEvents events) {
	this->events = events;
}

const AnimationId AnimationHandler::CreateAnimation(AnimationEvents events) {
	s_animations[s_animation_count] = std::make_unique<Animation>(events);
	return s_animation_count++;
}

InstanceId AnimationHandler::AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, AnimationEvents events) {
	
	auto de = s_animations.at(id)->events;

	AnimationInstance instance;
	instance.id = s_instace_count;
	instance.obj = obj;

	instance.events.onStart = events.onStart ? events.onStart : de.onStart;
	instance.events.onEachRepeatStart = events.onEachRepeatStart ? events.onEachRepeatStart : de.onEachRepeatStart;
	instance.events.onUpdate = events.onUpdate ? events.onUpdate : de.onUpdate;
	instance.events.onEachRepeatEnd = events.onEachRepeatEnd ? events.onEachRepeatEnd : de.onEachRepeatEnd;
	instance.events.onEnd = events.onEnd ? events.onEnd : de.onEnd;

	instance.duration = duration;
	instance.repeat = repeat;
	s_instances.push_back(instance);

	return s_instace_count++;
}

void AnimationHandler::UpdateAnimations(float dt) {
	for (auto& instance : s_instances) {

		auto& events = instance.events;

		if (instance.state == ANIM_PAUSED) continue;
		if (instance.state == ANIM_FINISHED) continue;

		if (instance.state == ANIM_STOPPING) {
			instance.state = ANIM_FINISHED;
			if (events.onEnd) events.onEnd();
			continue;
		}

		instance.time += dt;
		instance.time = instance.time > instance.duration ? instance.duration : instance.time;
		
		if (instance.state == ANIM_STARTING) {
			if (instance.events.onStart) instance.events.onStart();
			if (events.onEachRepeatStart) events.onEachRepeatStart(instance.obj);
			instance.state = ANIM_RUNNING;
		}

		if (events.onUpdate) events.onUpdate(instance.time / instance.duration, instance.obj);

		if (instance.time == instance.duration) {
			if (instance.repeat != 0) instance.repeat_count++;
			
			if (events.onEachRepeatEnd) events.onEachRepeatEnd(instance.obj);
			instance.time = 0.0f;

			if (instance.repeat > 0 && instance.repeat_count == instance.repeat) {
				instance.state = ANIM_FINISHED;
				if (events.onEnd) events.onEnd();
			} else {
				if (events.onEachRepeatStart) events.onEachRepeatStart(instance.obj);
			}
		}
	}
	s_instances.erase(
        std::remove_if(s_instances.begin(), s_instances.end(),
            [](const AnimationInstance& instance) {
                return instance.state == ANIM_FINISHED;
            }),
        s_instances.end()
    );
}

bool AnimationHandler::HasAnimation(AnimationId id) {
	return s_animations.find(id) != s_animations.end();
}

void AnimationHandler::RemoveAnimation(AnimationId id) {
	s_animations.erase(id);
}

void AnimationHandler::ClearAnimations() {
	s_animations.clear();
}

void AnimationHandler::Pause(InstanceId id) {
	for (auto& inst : s_instances) {
        if (inst.id == id) {
			inst.state = ANIM_PAUSED;
			return;
		}
    }
}

void AnimationHandler::Stop(InstanceId id) {
	for (auto& inst : s_instances) {
        if (inst.id == id) {
			inst.state = ANIM_STOPPING;
			return;
		} 
    }
}

void AnimationHandler::Continue(InstanceId id) {
	for (auto& inst : s_instances) {
        if (inst.id == id) {
			inst.state = ANIM_RUNNING;
			return;
		}
    }
}

void AnimationHandler::Restart(InstanceId id) {
	
	AnimationInstance* instance = nullptr;

	for (auto& inst : s_instances) {
        if (inst.id == id) {
			inst.state = ANIM_STARTING;
			inst.time = 0.0f;
			inst.repeat_count = 0;
			return;
		}
    }
}

size_t AnimationHandler::s_animation_count = 0;
std::unordered_map<AnimationId, std::unique_ptr<Animation>> AnimationHandler::s_animations = { };

size_t AnimationHandler::s_instace_count = 0;
std::vector<AnimationInstance> AnimationHandler::s_instances = { };

#ifdef ANIM_NAMESPACE
}
#endif

#endif