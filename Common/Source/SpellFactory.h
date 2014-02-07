#pragma once
#include "SpellInstance.h"

class SpellFactory
{
public:
	SpellFactory();
	~SpellFactory();

	void init();
	virtual SpellDefinition::ptr createSpellDefinition(const char* p_Filename, const char* p_Spellname);
	virtual SpellInstance::ptr createSpellInstance(SpellDefinition::ptr p_Spell);

	void readDefinitionFromFile(const char* p_Filename);


};
