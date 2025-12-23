#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>

typedef size_t AnimationId;

typedef std::function<void(float, float, void*)> UpdateFunction;
typedef std::function<void(void*)> ResetFunction;
typedef std::function<void(void)> AfterFunction;

class Animation {

public:

	Animation(UpdateFunction update);
	~Animation() = default;

	void Attach(void* obj, float duration, size_t repeat, ResetFunction reset, AfterFunction after);
	void Update(float dt);

private:

	UpdateFunction m_update = nullptr;
	
	struct AnimationInstance {
		void* obj = nullptr;
		ResetFunction reset = nullptr;
        AfterFunction after = nullptr;

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
	static const AnimationId CreateAnimation(UpdateFunction update);
    static void AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, ResetFunction reset = nullptr, AfterFunction after = nullptr);
    static void UpdateAnimations(float dt);
private:
    static size_t s_animation_count;
	static std::unordered_map<AnimationId, std::unique_ptr<Animation>> s_animations;
};

#ifdef ANIMATE_HPP_IMPLEMENTATION

Animation::Animation(UpdateFunction update) {
    m_update = update;
}

void Animation::Attach(void* obj, float duration, size_t repeat, ResetFunction reset, AfterFunction after) {
    AnimationInstance instance;
    instance.obj = obj;
    instance.reset = reset;
    instance.after = after;
    instance.duration = duration;
    instance.repeat = repeat;
    m_instances.push_back(instance);
}

void Animation::Update(float dt) {
    for (auto& instance : m_instances) {
        float moment = instance.time + dt;
        instance.time = moment > instance.duration ? instance.duration : moment;
        
        m_update(instance.duration, instance.time, instance.obj);

        if (instance.time == instance.duration) {
            instance.repeat_count++;
            
            if (instance.reset) instance.reset(instance.obj);
            instance.time = 0.0f;

            if (instance.repeat > 0 && instance.repeat_count == instance.repeat) {
                instance.finished = true;
                if (instance.after) instance.after();
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

const AnimationId AnimationHandler::CreateAnimation(UpdateFunction update) {
    s_animations[s_animation_count] = std::make_unique<Animation>(update);
    return s_animation_count++;
}

void AnimationHandler::AttachAnimation(AnimationId id, void* obj, float duration, size_t repeat, ResetFunction reset, AfterFunction after) {
    s_animations.at(id)->Attach(obj, duration, repeat, reset, after);
}

void AnimationHandler::UpdateAnimations(float dt) {
    for (const auto& [key, animation] : s_animations) {
        animation->Update(dt);
    }
}

size_t AnimationHandler::s_animation_count = 0;
std::unordered_map<AnimationId, std::unique_ptr<Animation>> AnimationHandler::s_animations = { };

#endif