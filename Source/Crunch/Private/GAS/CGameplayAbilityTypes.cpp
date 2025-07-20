// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CGameplayAbilityTypes.h"

FGenericDamageEffectDef::FGenericDamageEffectDef()
	:DamageEffect{ nullptr },
	PushVelocity{ 0.f }
{
}

FHeroBaseStats::FHeroBaseStats()
	:Class{ nullptr },
	Strength{ 0.f },
	Intelligence{ 0.f },
	StrengthGrowthRate{ 0.f },
	IntelligenceGrowthRate{ 0.f },
	BaseMaxHealth{ 0.f },
	BaseMaxMana(0.f),
	BaseAttackDamage{ 0.f },
	BaseArmor{ 0.f },
	BaseMoveSpeed{ 0.f }
{
}
