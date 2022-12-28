#include <SkyrimScripting/Plugin.h>

#include "NamedActorValues.h"

std::atomic<bool> ValuesPrinted;
std::vector<NamedActorValue> NamedActorValues;

void PrintActorValues(RE::Actor* actor) {
    auto actorInfo = std::format("{},{:x}", actor->GetActorBase()->GetName(), actor->GetFormID());
    auto actorValues = actor->GetActorRuntimeData().avStorage;
    for (auto& namedActorValue : NamedActorValues) {
        auto value = actorValues.baseValues[namedActorValue.value];
        if (value)
            actorInfo += std::format(",{}", *value);
        else
            actorInfo += ",?";
    }
    for (auto& namedActorValue : NamedActorValues) {
        auto value = actorValues.modifiers[namedActorValue.value]->modifiers;
        if (value)
            actorInfo += std::format(",{}", *value);
        else
            actorInfo += ",?";
    }
    logger::info("{}", actorInfo);
}

EventHandlers {
    On<RE::TESActivateEvent>([](const RE::TESActivateEvent*) {
        if (ValuesPrinted.exchange(true)) return;

        spdlog::set_pattern("%v");

        NamedActorValues = GetAllNamedActorValues();
        std::string header = "Name,FormID";
        for (auto& namedActorValue : NamedActorValues) header += ",Base " + namedActorValue.name;
        for (auto& namedActorValue : NamedActorValues) header += ",Mod " + namedActorValue.name;
        logger::info("{}", header);

        const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *literallyEveryFormInTheGame) {
            if (form->Is(RE::FormType::ActorCharacter)) {
                auto* actor = form->As<RE::Actor>();
                if (actor && actor->GetActorBase()->IsUnique()) PrintActorValues(actor);
            }
        }
    });
}
