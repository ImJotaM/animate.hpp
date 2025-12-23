#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>

class Animation {

public:

	Animation(std::function<void(float, float, void*)> update);
	~Animation() = default;

	void Attach(void* obj, float duration, size_t repeat, std::function<void(void*)> reset);
	void Update(float dt);

private:

	std::function<void(float, float, void*)> m_update = nullptr;
	
	struct AnimationInstance {
		void* obj = nullptr;
		std::function<void(void*)> reset = nullptr;

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
	static const std::string CreateAnimation(const std::string& id, std::function<void(float, float, void*)> update);
    static void AttachAnimation(const std::string& id, void* obj, float duration, size_t repeat, std::function<void(void*)> reset = nullptr);
    static void UpdateAnimations(float dt);
private:
	static std::unordered_map<std::string, std::shared_ptr<Animation>> s_animations;
};

#define ANIMATE_HPP_IMPLEMENTATION

#ifdef ANIMATE_HPP_IMPLEMENTATION

Animation::Animation(std::function<void(float, float, void*)> update) {
    m_update = update;
}

void Animation::Attach(void* obj, float duration, size_t repeat, std::function<void(void*)> reset) {
    AnimationInstance instance;
    instance.obj = obj;
    instance.reset = reset;
    instance.duration = duration;
    instance.repeat = repeat;
    m_instances.push_back(instance);
}

void Animation::Update(float dt) {
    for (auto& instance : m_instances) {
        float moment = instance.time + dt;
        instance.time = moment > instance.duration ? instance.duration : moment;

        if (instance.time == instance.duration) {
            instance.repeat_count++;
            
            if (instance.reset) instance.reset(instance.obj);
            instance.time = 0.0f;

            if (instance.repeat > 0 && instance.repeat_count == instance.repeat) instance.finished = true;
        }

        m_update(instance.duration, instance.time, instance.obj);
    }
    m_instances.erase(
        std::remove_if(m_instances.begin(), m_instances.end(),
            [](const AnimationInstance& instance) {
                return instance.finished;
            }),
        m_instances.end()
    );
}

const std::string AnimationHandler::CreateAnimation(const std::string& id, std::function<void(float, float, void*)> update) {
    s_animations[id] = std::make_shared<Animation>(update);
    return id;
}

void AnimationHandler::AttachAnimation(const std::string& id, void* obj, float duration, size_t repeat, std::function<void(void*)> reset = nullptr) {
    s_animations.at(id)->Attach(obj, duration, repeat, reset);
}

void AnimationHandler::UpdateAnimations(float dt) {
    for (const auto& [key, animation] : s_animations) {
        animation->Update(dt);
    }
}

std::unordered_map<std::string, std::shared_ptr<Animation>> AnimationHandler::s_animations = { };

#endif