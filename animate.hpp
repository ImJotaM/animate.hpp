#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>
#include <cstdint>

#ifdef ANIM_NAMESPACE
namespace Anim {
#endif

typedef size_t AnimationId;

struct _MapHandleSlot {
    uint32_t slot_index = 0;
    uint32_t generation = 0;
};

template <typename T>
class _SlotMap {

public:

    _MapHandleSlot insert(T value) {

        uint32_t slot_index;

        if (!m_free_slots.empty()) {
            slot_index = m_free_slots.back();
            m_free_slots.pop_back();
        } else {
            slot_index = (uint32_t)m_slots.size();
            m_slots.push_back({ });
        }

        m_slots[slot_index].active = true;
        m_slots[slot_index].data_index = m_data.size();

        m_data.push_back({ value, slot_index });

        return { slot_index, m_slots[slot_index].generation };
    }

    bool is_valid(_MapHandleSlot handle) const {
        return 
        handle.slot_index < m_slots.size() &&
        m_slots[handle.slot_index].active &&
        m_slots[handle.slot_index].generation == handle.generation;
    }

    T* get(_MapHandleSlot handle) {
        if (!is_valid(handle)) return nullptr;
        return &m_data[m_slots[handle.slot_index].data_index].value;
    }

    void erase(_MapHandleSlot handle) {
        if (!is_valid(handle)) return;

        uint32_t slot_index = handle.slot_index;
        uint32_t data_index = m_slots[handle.slot_index].data_index;

        m_slots[slot_index].active = false;
        m_slots[slot_index].generation++;
        m_free_slots.push_back(slot_index);

        if (data_index < (uint32_t)m_data.size() - 1) {
            m_data[data_index] = std::move(m_data.back());
            m_slots[m_data[data_index].slot_index].data_index = data_index;
        }

        m_data.pop_back();

    }

	size_t size() const { return m_data.size(); }
	T& operator[](size_t index) { return m_data[index].value; }

	_MapHandleSlot get_handle_at(size_t index) {
		return { m_data[index].slot_index, m_slots[m_data[index].slot_index].generation };
	}

private:

    struct Item {
        T value;
        uint32_t slot_index = 0;
    };

    struct Slot {
        uint32_t data_index = 0;
        uint32_t generation = 0;
        bool active = false;
    };

    std::vector<Item> m_data = { };
    std::vector<Slot> m_slots = { };
    std::vector<uint32_t> m_free_slots = { };
    
};

typedef _MapHandleSlot InstanceId;

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

	InstanceId id = { };

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

	static _SlotMap<AnimationInstance> s_instances;

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
	
	instance.obj = obj;

	instance.events.onStart = events.onStart ? events.onStart : de.onStart;
	instance.events.onEachRepeatStart = events.onEachRepeatStart ? events.onEachRepeatStart : de.onEachRepeatStart;
	instance.events.onUpdate = events.onUpdate ? events.onUpdate : de.onUpdate;
	instance.events.onEachRepeatEnd = events.onEachRepeatEnd ? events.onEachRepeatEnd : de.onEachRepeatEnd;
	instance.events.onEnd = events.onEnd ? events.onEnd : de.onEnd;

	instance.duration = duration;
	instance.repeat = repeat;

	return s_instances.insert(instance);
}

void AnimationHandler::UpdateAnimations(float dt) {
	for (size_t i = 0; i < s_instances.size(); ) {

		auto& instance = s_instances[i];

		auto& events = instance.events;

		if (instance.state == ANIM_PAUSED) { ++i; continue;}

		if (instance.state == ANIM_STOPPING) {
			if (events.onEnd) events.onEnd();
			s_instances.erase(s_instances.get_handle_at(i));
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
				s_instances.erase(s_instances.get_handle_at(i));
			} else {
				if (events.onEachRepeatStart) events.onEachRepeatStart(instance.obj);
				++i;
			}
		} else {
			++i;
		}
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

void AnimationHandler::Pause(InstanceId id) {
	if (auto* inst = s_instances.get(id)) inst->state = ANIM_PAUSED;
}

void AnimationHandler::Stop(InstanceId id) {
	if (auto* inst = s_instances.get(id)) inst->state = ANIM_STOPPING;
}

void AnimationHandler::Continue(InstanceId id) {
	if (auto* inst = s_instances.get(id)) inst->state = ANIM_RUNNING;
}

void AnimationHandler::Restart(InstanceId id) {
	if (auto* inst = s_instances.get(id)) {
		inst->state = ANIM_STARTING;
		inst->time = 0.0f;
		inst->repeat_count = 0;
	}
}

size_t AnimationHandler::s_animation_count = 0;
std::unordered_map<AnimationId, std::unique_ptr<Animation>> AnimationHandler::s_animations = { };

_SlotMap<AnimationInstance> AnimationHandler::s_instances;

#ifdef ANIM_NAMESPACE
}
#endif

#endif