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

class Animation {

public:

	Animation(AnimationEvents events);
	~Animation() = default;

	void Attach(void* obj, float duration, size_t repeat, AnimationEvents events);
	void Update(float dt);

private:

	AnimationEvents m_events = { };
	
	struct AnimationInstance {
		void* obj = nullptr;
		AnimationEvents events = { };

		float duration = 0.0f;
		size_t repeat = 0;
		
		float time = 0.0f;
		size_t repeat_count = 0;
		bool finished = false;
	};

	std::vector<AnimationInstance> m_instances;

};

class AnimationHandler {

public:
	static const AnimationId CreateAnimation(AnimationEvents update);
    static void AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, AnimationEvents events);
    static void UpdateAnimations(float dt);

    static bool HasAnimation(AnimationId id);
    static void RemoveAnimation(AnimationId id);
    static void ClearAnimations();

private:
    static size_t s_animation_count;
	static std::unordered_map<AnimationId, std::unique_ptr<Animation>> s_animations;
};

#ifdef ANIM_NAMESPACE
}
#endif

#ifdef ANIMATE_HPP_IMPLEMENTATION

#ifdef ANIM_NAMESPACE
namespace Anim {
#endif

Animation::Animation(AnimationEvents events) {
    m_events = events;
}

void Animation::Attach(void* obj, float duration, size_t repeat, AnimationEvents events) {
    AnimationInstance instance;
    instance.obj = obj;

    instance.events.onStart = events.onStart ? events.onStart : m_events.onStart;
    instance.events.onEachRepeatStart = events.onEachRepeatStart ? events.onEachRepeatStart : m_events.onEachRepeatStart;
    instance.events.onUpdate = events.onUpdate ? events.onUpdate : m_events.onUpdate;
    instance.events.onEachRepeatEnd = events.onEachRepeatEnd ? events.onEachRepeatEnd : m_events.onEachRepeatEnd;
    instance.events.onEnd = events.onEnd ? events.onEnd : m_events.onEnd;

    instance.duration = duration;
    instance.repeat = repeat;
    m_instances.push_back(instance);
}

void Animation::Update(float dt) {
    for (auto& instance : m_instances) {
        
        auto& events = instance.events;

        instance.time += dt;
        instance.time = instance.time > instance.duration ? instance.duration : instance.time;
        
        if (instance.repeat_count == 0 && instance.events.onStart) instance.events.onStart();
        if (events.onEachRepeatStart) events.onEachRepeatStart(instance.obj);

        if (events.onUpdate) events.onUpdate(instance.time / instance.duration, instance.obj);

        if (instance.time == instance.duration) {
            instance.repeat_count++;
            
            if (events.onEachRepeatEnd) events.onEachRepeatEnd(instance.obj);
            instance.time = 0.0f;

            if (instance.repeat > 0 && instance.repeat_count == instance.repeat) {
                instance.finished = true;
                if (events.onEnd) events.onEnd();
            } else if (instance.repeat == 0) {
                if (events.onEnd) events.onEnd();
            }
        }
    }
    m_instances.erase(
        std::remove_if(m_instances.begin(), m_instances.end(),
            [](const AnimationInstance& instance) {
                return instance.finished;
            }),
        m_instances.end()
    );
}

const AnimationId AnimationHandler::CreateAnimation(AnimationEvents update) {
    s_animations[s_animation_count] = std::make_unique<Animation>(update);
    return s_animation_count++;
}

void AnimationHandler::AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, AnimationEvents events) {
    s_animations.at(id)->Attach(obj, duration, repeat, events);
}

void AnimationHandler::UpdateAnimations(float dt) {
    for (const auto& [key, animation] : s_animations) {
        animation->Update(dt);
    }
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

size_t AnimationHandler::s_animation_count = 0;
std::unordered_map<AnimationId, std::unique_ptr<Animation>> AnimationHandler::s_animations = { };

#ifdef ANIM_NAMESPACE
}
#endif

#endif