#include "saved_machine.h"
#include <iostream>
#include <fstream>

namespace AbstractFSM
{
	namespace DataDriven
	{
#pragma region Member
		SavedMachine::SavedMachine()
		{
			try
			{
				ImportFromXML();
			}
			catch ( std::exception ex )
			{
				m_states_.clear();
			}

			// Create default map if no data exists and then create states
			if ( m_states_.size() == 0 )
			{
				GenerateDefaultMap();
				ImportFromXML();
			}

			// Find the entry state
			for ( size_t i = 0, len = m_states_.size(); i < len; ++i )
			{
				if ( m_states_[i]->IsStartState() )
				{
					m_current_ = m_states_[i];
					break;
				}
			}
			if ( m_current_ == NULL )
			{
				std::cout << "\n\nERROR! NO ENTRY STATE DEFINED.";
				throw new std::exception("No entry state defined in this state machine.  Cannot continue.");
			}
		}
		// Create map if XML does not exist
		void SavedMachine::GenerateDefaultMap()
		{
			m_states_.clear();

			// Create all the fun states in our mini-world
			m_states_.push_back(new SavedMachineState("entry_hall", SavedMachineState::Enter, "Grand Entrance", "You are standing in a grand enterance of a castle.\nThere are tables and chairs, but nothing you can interact with.", "staircase,outside"));
			m_states_.push_back(new SavedMachineState("staircase", SavedMachineState::None, "Grand Staircase", "The staircase is made from beautiful granite.", "east_wing,west_wing,entry_hall"));
			m_states_.push_back(new SavedMachineState("east_wing", SavedMachineState::None, "East Wing", "This wing is devoted to bedrooms.", "bedroom_A,bedroom_B,bedroom_C,staircase"));
			m_states_.push_back(new SavedMachineState("west_wing", SavedMachineState::None, "West Wing", "This wing is devoted to business.", "workroom_A,workroom_B,workroom_C"));
			m_states_.push_back(new SavedMachineState("bedroom_A", SavedMachineState::None, "Master Suite", "This is the master suite.  What a fancy room.", "east_wing"));
			m_states_.push_back(new SavedMachineState("bedroom_B", SavedMachineState::None, "Prince Bob's Room", "The prince has an extensive library on his wall.\nHe also has more clothes than most males know what to do with.", "east_wing"));
			m_states_.push_back(new SavedMachineState("bedroom_C", SavedMachineState::None, "Princess Alice's Room", "The princess has filled her room with a small compur lab.\nShe spends her days playing games and writing code.", "east_wing"));
			m_states_.push_back(new SavedMachineState("workroom_A", SavedMachineState::None, "Study", "This is the study.  It has many books.", "west_wing"));
			m_states_.push_back(new SavedMachineState("workroom_B", SavedMachineState::None, "Bathroom", "Every home needs one", "west_wing"));
			m_states_.push_back(new SavedMachineState("workroom_C", SavedMachineState::None, "Do Not Enter", "I warned you not to enter.\nYou are in a maze of twisty little passages, all alike.", "passage"));
			m_states_.push_back(new SavedMachineState("passage", SavedMachineState::None, "Twisty Passage", "You are in a maze of twisty little passages, all alike", "passage"));
			m_states_.push_back(new SavedMachineState("outside", SavedMachineState::Exit, "Outside", "You have successfully exited the castle.", ""));

			ExportToXML();
		}
#pragma endregion

#pragma region Helper Functions
		// Write into the XML file
		void SavedMachine::ExportToXML()
		{
			rapidxml::xml_document<> doc;
			xml_node<> * root_node = doc.allocate_node(rapidxml::node_type::node_element, "SavedMachine");
			doc.append_node(root_node);
			WriteXml(doc);
			std::ofstream xmlFile("GameRooms.xml");
			xmlFile << doc;
			xmlFile.close();
			doc.clear();
		}
		// Read the XML file
		void SavedMachine::ImportFromXML()
		{
			rapidxml::file<> xmlFile("GameRooms.xml");
			rapidxml::xml_document<> doc;
			doc.parse<0>(xmlFile.data());
			ReadXml(doc);
		}
		// Populate the states based on the DOM tree
		void SavedMachine::ReadXml(xml_document<> &doc)
		{
			xml_node<> * root_node = doc.first_node("SavedMachine");
			bool isEmpty = root_node == NULL ? true : false;
			if ( isEmpty ) return;
			for ( xml_node<> * room_node = root_node->first_node("Room"); room_node; room_node = room_node->next_sibling() )
			{
				m_states_.push_back(new SavedMachineState(room_node));
			}
		}
		// Convert the existing state data into a DOM tree
		void SavedMachine::WriteXml(xml_document<> &doc)
		{
			for ( size_t i = 0, len = m_states_.size(); i < len; ++i )
			{
				m_states_[i]->WriteXml(doc);
			}
		}
#pragma endregion

#pragma region StateMachine Overrides
		// Accessor to look at the current state
		State * SavedMachine::CurrentState() const
		{
			return m_current_;
		}
		// List of all possible transitions we can make from this current state
		std::vector<std::string> SavedMachine::PossibleTransitions()
		{
			std::vector<std::string> result;
			for ( size_t i = 0, len = m_current_->Neighbors().size(); i < len; ++i )
			{
				for ( size_t j = 0, len = m_states_.size(); j < len; ++j )
				{
					if ( m_current_->Neighbors()[i] == m_states_[j]->Key() )
					{
						result.push_back(m_states_[j]->GetName());
						break;
					}
				}
			}
			return result;
		}
		// Advance to the specified machine state
		bool SavedMachine::Advance(std::string nextState)
		{
			for ( size_t i = 0, len = m_states_.size(); i < len; ++i )
			{
			    if ( m_states_[i]->IsMyName(nextState) && std::find(m_current_->Neighbors().begin(), m_current_->Neighbors().end(), m_states_[i]->Key()) != m_current_->Neighbors().end())
				{
					m_current_ = m_states_[i];
					return true;
				}
			}
			std::cout << "Cannot do that.";
			return false;
		}
		// Check if machine has reached exit node
		bool SavedMachine::IsComplete()
		{
			return m_current_->IsExitState();
		}
#pragma endregion
	}
}