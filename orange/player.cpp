/*
	Copyright 2008-2009 Ambient.5

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include ".\\utils.h"
#include "..\\Deathway\\SimpleModulus\\SimpleModulus.h"
#include ".\\DataBase.h"
#include ".\\packets.h"
#include ".\\ItemManager.h"
#include ".\\objectmanager.h"
#include ".\\player.h"

const CItem dummy;

CPlayer::CPlayer()
{
	this->socket = NULL;
	this->status = PLAYER_EMPTY;
	this->type = OBJECT_PLAYER;
	this->guid = -1;
	this->tick_count = GetTickCount();
	this->last_save_time = GetTickCount();
	this->failed_attempts = NULL;
	for(int i = 0; i < 108; ++i)
	{
		this->inventory[i] = (CItem*)&dummy;
	}
	this->send_serial = NULL;
	this->viewport.resize(100);
	this->viewport.clear();
	this->tick_count = GetTickCount();
	this->last_move_time = GetTickCount();
	this->check_time = GetTickCount();
	this->pklevel = 0;
	this->rest = 0;

	this->experience = 0;
	this->leveluppoint = 0;
	this->money = 0;
	this->pklevel = 0;
	this->gmlevel = 0;
	this->addpoint = 0;
	this->maxaddpoint = 0;
	this->minuspoint = 0;
	this->maxminuspoint = 0;
	this->Class = 0;
	this->changeup = 0;

	ZeroMemory(this->account, sizeof(this->account));
	ZeroMemory(this->name, sizeof(this->name));
	ZeroMemory(this->charset, sizeof(this->charset));
}

void CPlayer::Send(unsigned char* buffer, size_t len)
{
	unsigned char in_buffer[16384];
	unsigned char send_buffer[16384];
	ZeroMemory(in_buffer, sizeof(in_buffer));
	ZeroMemory(send_buffer, sizeof(send_buffer));
	memcpy(in_buffer, buffer, len);

	int ret;
	unsigned char btsize;

	unsigned char code = buffer[0];
	switch(code)
	{
	case 0xC1:
		{
			this->socket->SendBuf((const char*)in_buffer, len);
			break;
		}
	case 0xC2:
		{
			this->socket->SendBuf((const char*)in_buffer, len);
			break;
		}
	case 0xC3:
		{
			btsize = in_buffer[1];
			in_buffer[1] = this->send_serial;
			this->send_serial++;

			ret = g_SimpleModulusSC.Encrypt(&send_buffer[2], &in_buffer[1], len - 1);
			send_buffer[0] = 0xC3;
			send_buffer[1] = ret + 2;
			size_t size = ret + 2;
			in_buffer[1] = btsize;
			this->socket->SendBuf((const char*)send_buffer, size);
			break;
		}
	case 0xC4:
		{
			btsize = in_buffer[2];
			in_buffer[2] = this->send_serial;
			this->send_serial++;
			ret = g_SimpleModulusSC.Encrypt(&send_buffer[3], &in_buffer[2], len - 2);
			send_buffer[0] = 0xC4;
			send_buffer[1] = HIBYTE(ret + 3);
			send_buffer[2] = LOBYTE(ret + 3);
			size_t size = ret + 3;
			in_buffer[2] = btsize;
			this->socket->SendBuf((const char*)send_buffer, size);
			break;
		}
	default:
		{
			break;
		}
	}
}

void CPlayer::Close()
{
	ServerSocket * s = this->socket;
	TcpSocket * tcp = dynamic_cast<TcpSocket *>(s);
	if(tcp)
	{
		tcp->SetCloseAndDelete();
	}
}

void CPlayer::SetStatus(unsigned char status)
{
	QSqlQuery q;
	MainDB.Lock();
	q.prepare("UPDATE `account_test` SET `status` = :status WHERE `account` = ':account'");
	q.bindValue(":status", status);
	q.bindValue(":account", this->account);
	q.exec();
	MainDB.Unlock();
}

int CPlayer::LoadCharacters()
{
	char seps[] = " ";
	ZeroMemory(this->charinfo, sizeof(DATA_CHARINFO) * 5);
	QSqlQuery q;
	int count = 0;
	MainDB.Lock();
	q.prepare("SELECT `name`, `class`, `changeup`, `position`, `experience`, `leveluppoint`, `level`, `strength`, `dexterity`, `vitality`, `energy`, `leadership`, `life`, `mana`, `shield`, `bp`, `money`, `pklevel`, `gmlevel`, `addpoint`, `maxaddpoint`, `minuspoint`, `maxminuspoint`, `inventory_guids`, `spell_data`, `guild_data` FROM `characters` WHERE `account` = ':account'");
	q.bindValue(":account", this->account);
	q.exec();
	while((q.next()) && (count <= 4))
	{
		strcpy_s(this->charinfo[count].Account, 11, this->account);
		strcpy_s(this->charinfo[count].Name, 11, q.value(0).toString().toStdString().c_str());
		this->charinfo[count].Class = q.value(1).toUInt();
		this->charinfo[count].ChangeUp = q.value(2).toUInt();
		this->charinfo[count].Position = q.value(3).toUInt();
		this->charinfo[count].Exp = q.value(4).toULongLong();
		this->charinfo[count].LevelUpPoint = q.value(5).toUInt();
		this->charinfo[count].Level = q.value(6).toUInt();
		this->charinfo[count].Str = q.value(7).toUInt();
		this->charinfo[count].Dex = q.value(8).toUInt();
		this->charinfo[count].Vit = q.value(9).toUInt();
		this->charinfo[count].Energy = q.value(10).toUInt();
		this->charinfo[count].Leadership = q.value(11).toUInt();
		this->charinfo[count].Life = q.value(12).toUInt();
		this->charinfo[count].Mana = q.value(13).toUInt();
		this->charinfo[count].Shield = q.value(14).toUInt();
		this->charinfo[count].BP = q.value(15).toUInt();
		this->charinfo[count].Money = q.value(16).toUInt();
		this->charinfo[count].PkLevel = q.value(17).toUInt();
		this->charinfo[count].GMLevel = q.value(18).toUInt();
		this->charinfo[count].AddPoint = q.value(19).toUInt();
		this->charinfo[count].MaxAddPoint = q.value(20).toUInt();
		this->charinfo[count].MinusPoint = q.value(21).toUInt();
		this->charinfo[count].MaxMinusPoint = q.value(22).toUInt();
		std::string inventory_guids = q.value(23).toString().toStdString();
		this->charinfo[count].item_guids.clear();
		char* token = strtok((char*)inventory_guids.c_str(), seps);
		while(token != NULL)
		{
			int guid;
			sscanf_s(token, "%d", &guid);
			this->charinfo[count].item_guids.push_back(guid);
			token = strtok(NULL, seps);
		}
		//memcpy(this->charinfo[count].Spell_data, (q->getstr()), 1);
		//memcpy(this->charinfo[count].Guild_data, (q->getstr()), 1);
		QString stri1 = q.value(24).toString();
		QString stri2 = q.value(25).toString();
		count++;
	}
	MainDB.Unlock();
	for(uint32 i = 0; i < count; ++i) //12 items for viewport
	{
		printf_s("guids %d\n", this->charinfo[i].item_guids.size());
		for(uint32 n = 0; n < this->charinfo[i].item_guids.size(); ++n)
		{
			DATA_ITEM ditem;
			bool result = CItemManager::LoadItem(&ditem, this->charinfo[i].item_guids.at(n));
			if(result && ditem.slot < 12)
			{
				memcpy(&this->charinfo[i].temp_inv[ditem.slot], &ditem, sizeof(DATA_ITEM));
			}
		}
	}
	return count;
}

void CPlayer::SendInventory()
{
	PMSG_INVENTORYLISTCOUNT phead;
	ZeroMemory(&phead, sizeof(PMSG_INVENTORYLISTCOUNT));
	unsigned char buffer[4096];
	ZeroMemory(buffer, sizeof(buffer));
	size_t offs = sizeof(PMSG_INVENTORYLISTCOUNT);
	int count = 0;
	for(int i = 0; i < 108; ++i)
	{
		CItem * item = this->inventory[i];
		if(item->IsItem())
		{
			count++;
			PMSG_INVENTORYLIST data;
			data.Pos = i;
			ItemByteConvert(data.ItemInfo, item->type, item->m_Option1, item->m_Option2, item->m_Option3, (unsigned char)item->level, (unsigned char)item->durability, item->m_NewOption, item->m_SetOption, item->m_JewelOfHarmonyOption, item->m_ItemOptionEx);
			memcpy(&buffer[offs], &data, sizeof(PMSG_INVENTORYLIST));
			offs += sizeof(PMSG_INVENTORYLIST);
		}
	}
	phead.h.c = 0xC4;
	phead.h.sizeH = HIBYTE(offs);
	phead.h.sizeL = LOBYTE(offs);
	phead.h.headcode = 0xF3;
	phead.subcode = 0x10;
	phead.Count = count;
	memcpy(buffer, &phead, sizeof(PMSG_INVENTORYLISTCOUNT));
	this->Send(buffer, offs);
}

void CPlayer::AssignItem(DATA_ITEM *data)
{
	CItem * item =  this->inventory[data->slot];
	/*if(!ItemManager.Instanciate(item))
	{
		item->type = -1;
		return;
	}*/
	item->guid = data->guid;
	item->type = data->type;
	item->level = data->level;
	item->durability = (float)data->durability;
	item->m_Option1 = data->option1;
	item->m_Option2 = data->option2;
	item->m_Option3 = data->option3;
	item->m_NewOption = data->newoption;
	item->m_SetOption = data->setoption;
	item->m_JewelOfHarmonyOption = data->joh_option;
	item->m_ItemOptionEx = data->optionex;
	item->m_PetItem_Exp = data->petitem_exp;
	item->m_PetItem_Level = data->petitem_level;
}

bool CPlayer::InViewport(CObject *obj)
{
	for(uint32 i = 0; i < this->viewport.size(); ++i)
	{
		if((ObjManager.FindByGuid(this->viewport.at(i)) == obj) && (obj != NULL) && (obj->type > OBJECT_EMPTY))
		{
			return true;
		}
	}
	return false;
}

/*void CPlayer::DeleteFromViewport(void* obj)
{
	for(uint32 i = 0; i < this->viewport.size(); ++i)
	{
		if(obj == this->viewport.at(i))
		{
			this->viewport.erase(this->viewport.begin() + i);
			printf_s("deleted %p\n", obj);
			return;
		}
	}
}*/

void CPlayer::SendToViewport(unsigned char* buffer, size_t len)
{
	for(uint32 i = 0; i < this->viewport.size(); ++i)
	{
		CObject* object = ObjManager.FindByGuid(this->viewport.at(i));
		if((object) && (object->type == OBJECT_PLAYER))
		{
			((CPlayer*)object)->Send(buffer, len);
		}
	}
}

bool CPlayer::CheckPosition()
{
	if((abs(this->target_x - this->x_old) < 15) && (abs(this->target_y - this->y_old) < 15))
	{
		return true;
	}
	return false;
}

bool CPlayer::CheckPacketTime()
{
	if((GetTickCount() - this->check_time) >= 300)
	{
		return true;
	}
	return false;
}

void CPlayer::SetPosition(uint8 _x, uint8 _y)
{
	if((this->type != OBJECT_PLAYER) || (!this->CheckPacketTime()) || (this->teleporting))
	{
		return;
	}
	this->x = _x;
	this->y = _y;
	if(!((this->map >= 18) && (this->map <= 23))) //ChaosCastle Specific check here (blow time)
	{
		PMSG_RECV_POSISTION_SET data;
		data.h.c = 0xC1;
		data.h.size = sizeof(PMSG_RECV_POSISTION_SET);
		data.h.headcode = GC_RECV_POSITION_SET;
		data.NumberH = HIBYTE(this->guid);
		data.NumberL = LOBYTE(this->guid);
		data.X = _x;
		data.Y = _y;
		this->target_x = _x;
		this->target_y = _y;
		if(this->CheckPosition())
		{
			this->Send((uint8*)&data, sizeof(PMSG_RECV_POSISTION_SET));
			this->SendToViewport((uint8*)&data, sizeof(PMSG_RECV_POSISTION_SET));
			this->x_old = this->target_x;
			this->y_old = this->target_y;
		}
	}
}

bool CPlayer::SavePlayer()
{
	if(this->type != OBJECT_PLAYER)
	{
		return false;
	}
	this->last_save_time = GetTickCount();
	uint32 position = this->dir;
	position |= this->map * 0x100;
	position |= this->y * 0x10000;
	position |= this->x * 0x1000000;
	QSqlQuery q;
	MainDB.Lock();
	q.prepare("UPDATE `characters` SET `class` = :class, `changeup` = :changeup, `position` = :position, `experience` = :experience, `leveluppoint` = :leveluppoint, `level` = :level, `strength` = :strength, `dexterity` = :dexterity, `vitality` = :vitality, `energy` = :energy, `leadership` = :leadership, `life` = :life, `mana` = :mana, `shield` = :shield, `bp` = :bp, `money` = :money, `pklevel` = :pklevel, `gmlevel` = :gmlevel, `addpoint` = :addpoint, `maxaddpoint` = :maxaddpoint, `minuspoint` = :minuspoint, `maxminuspoint` = :maxminuspoint WHERE `account` = ':account' AND `name` = ':name';");
	q.bindValue(":class", this->Class);
	q.bindValue(":changeup", this->changeup);
	q.bindValue(":position", position);
	q.bindValue(":experience", this->experience);
	q.bindValue(":leveluppoint", this->leveluppoint);
	q.bindValue(":level", this->level);
	q.bindValue(":strength", this->strength);
	q.bindValue(":dexterity", this->dexterity);
	q.bindValue(":vitality", this->vitality);
	q.bindValue(":energy", this->energy);
	q.bindValue(":leadership", this->leadership);
	q.bindValue(":life", (uint32)this->life);
	q.bindValue(":mana", (uint32)this->mana);
	q.bindValue(":shield", (uint32)this->shield);
	q.bindValue(":bp", (uint32)this->bp);
	q.bindValue(":money", this->money);
	q.bindValue(":pklevel", this->pklevel);
	q.bindValue(":gmlevel", this->gmlevel);
	q.bindValue(":addpoint", this->addpoint);
	q.bindValue(":maxaddpoint", this->maxaddpoint);
	q.bindValue(":minuspoint", this->minuspoint);
	q.bindValue(":maxminuspoint", this->maxminuspoint);
	q.bindValue(":account", this->account);
	q.bindValue(":name", this->name);
	bool result = q.exec();
	MainDB.Unlock();
	if(result)
	{
		std::string inv;
		inv.clear();
		for(uint32 i = 0; i < 108; ++i)
		{
			if(this->inventory[i]->IsItem())
			{
				char temp[16];
				ZeroMemory(temp, sizeof(temp));
				sprintf_s(temp, sizeof(temp), "%u ", this->inventory[i]->guid);
				if(ItemManager.SaveItem(this->inventory[i], i))
				{
					inv.append(temp);
				}
			}
		}
		MainDB.Lock();
		q.prepare("UPDATE `characters` SET `inventory_guids` = ':inv' WHERE `account` = ':acc' AND `name` = ':name';");
		q.bindValue(":inv", inv.c_str());
		q.bindValue(":acc", this->account);
		q.bindValue(":name", this->name);
		result = q.exec();
		MainDB.Unlock();
		if(!result)
		{
			printf_s("Inventory save failed %s:%s:%u\n", this->account, this->name, q.lastError().type());
		}
	}
	else
	{
		printf_s("Character save failed %s:%s\n", this->account, this->name);
		return false;
	}
	return true;
}

void CPlayer::CookCharset()
{
	ZeroMemory(charset, 18);
	this->charset[0] = (this->Class * 0x20) & 0xE0; //0x100 - 0x20 = 0xE0
	this->charset[0] |= (this->changeup * 0x10) & 0x10;
	if(this->action == 0x80)
	{
		this->charset[0] |= 0x02;
	}
	else if(this->action == 0x81)
	{
		this->charset[0] |= 0x03;
	}
	if(this->inventory[WEAPON_01]->type >= 0)
	{
		this->charset[12] |= (this->inventory[WEAPON_01]->type & 0x0f00) / 0x10;  //12 char - highest 4 bits
		this->charset[1] = (this->inventory[WEAPON_01]->type & 0xff); //1 char both 4-bit fields
	}
	else //or -1;
	{
		this->charset[12] |= 0xf0;
		this->charset[1] = 0xff;
	}
	if(this->inventory[WEAPON_02]->type >= 0)
	{
		this->charset[13] |= (this->inventory[WEAPON_02]->type & 0x0f00) / 0x10;
		this->charset[2] = (this->inventory[WEAPON_02]->type & 0xff);
	}
	else
	{
		this->charset[13] |= 0xf0;
		this->charset[2] = 0xff;
	}
	if(this->inventory[HELMET]->type >= 0)
	{
		this->charset[13] |= (this->inventory[HELMET]->type & 0x01E0) / 0x20;
		this->charset[9] |= (this->inventory[HELMET]->type & 0x10) * 0x08;
		this->charset[3] |= (this->inventory[HELMET]->type & 0x0f) * 0x10;
	}
	else
	{
		this->charset[13] |= 0x0f;
		this->charset[9] |= 0x80;
		this->charset[3] |= 0xf0;
	}
	if(this->inventory[ARMOR]->type >= 0)
	{
		this->charset[14] |= (this->inventory[ARMOR]->type & 0x01E0) / 0x02;
		this->charset[9] |= (this->inventory[ARMOR]->type & 0x10) * 0x04;
		this->charset[3] |= (this->inventory[ARMOR]->type &0x0f);
	}
	else
	{
		this->charset[14] |= 0xf0;
		this->charset[9] |= 0x40;
		this->charset[3] |= 0x0f;
	}
	if(this->inventory[PANTS]->type >= 0)
	{
		this->charset[14] |= (this->inventory[PANTS]->type & 0x01E0) / 0x20;
		this->charset[9] |= (this->inventory[PANTS]->type & 0x10) * 0x02;
		this->charset[4] |= (this->inventory[PANTS]->type & 0x0f) * 0x10;
	}
	else
	{
		this->charset[14] |= 0x0f;
		this->charset[9] |= 0x20;
		this->charset[4] |= 0xf0;
	}
	if(this->inventory[GLOVES]->type >= 0)
	{
		this->charset[15] |= (this->inventory[GLOVES]->type & 0x01E0) / 0x02;
		this->charset[9] |= (this->inventory[GLOVES]->type & 0x10);
		this->charset[4] |= (this->inventory[GLOVES]->type & 0x0f);
	}
	else
	{
		this->charset[15] |= 0xf0;
		this->charset[9] |= 0x10;
		this->charset[4] |= 0x0f;
	}
	if(this->inventory[BOOTS]->type >= 0)
	{
		this->charset[15] |= (this->inventory[BOOTS]->type & 0x01E0) / 0x20;
		this->charset[9] |= (this->inventory[BOOTS]->type & 0x10) / 0x02;
		this->charset[5] |= (this->inventory[BOOTS]->type & 0x0f) * 0x10;
	}
	else
	{
		this->charset[15] |= 0x0f;
		this->charset[9] |= 0x08;
		this->charset[5] |= 0xf0;
	}
	uint8 index = 0;
	if(this->inventory[WINGS]->type >= 0)
	{
		index |= (this->inventory[WINGS]->type & 0x03) * 0x04;
	}
	else
	{
		index |= 0x0c;
	}
	if((this->inventory[GUARDIAN]->type >= 0) && !(this->inventory[GUARDIAN]->type == 6660))
	{
		index |= (this->inventory[GUARDIAN]->type & 0x03);
	}
	else
	{
		index |= 0x03;
	}
	this->charset[5] |= index;
	uint32 levelindex = 0;
	levelindex = LevelConvert(this->inventory[WEAPON_01]->level) & 0xff;
	levelindex |= (LevelConvert(this->inventory[WEAPON_02]->level) & 0xff) * 0x08;
	levelindex |= (LevelConvert(this->inventory[HELMET]->level) & 0xff) * 0x40;
	levelindex |= (LevelConvert(this->inventory[ARMOR]->level) & 0xff) * 0x200;
	levelindex |= (LevelConvert(this->inventory[PANTS]->level) & 0xff) * 0x1000;
	levelindex |= (LevelConvert(this->inventory[GLOVES]->level) & 0xff) * 0x8000;
	levelindex |= (LevelConvert(this->inventory[BOOTS]->level) & 0xff) * 0x40000;
	this->charset[6] = (levelindex / 0x10000) & 0xff;
	this->charset[7] = (levelindex / 0x100) & 0xff;
	this->charset[8] = (levelindex) & 0xff;
	if(((this->inventory[WINGS]->type >= (12 * 512 + 3)) && (this->inventory[WINGS]->type <= (12 * 512 + 6))) || (this->inventory[WINGS]->type == (13 * 512 + 30)))
	{
		this->charset[5] |= 0x0C;
		if(this->inventory[WINGS]->type == (13 * 512 + 30))
		{
			this->charset[5] |= 0x05;
		}
		else
		{
			this->charset[9] |= (this->inventory[WINGS]->type - 2) & 0x07;
		}
	}
	this->charset[10] = 0;
	/*if(lpObj->pInventory[HELMET].IsExtItem())
	{
		lpObj->CharSet[10] = 0x80;
	}
	if(lpObj->pInventory[ARMOR].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x40;
	}
	if(lpObj->pInventory[PANTS].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x20;
	}
	if(lpObj->pInventory[GLOVES].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x10;
	}
	if(lpObj->pInventory[BOOTS].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x8;
	}
	if(lpObj->pInventory[WEAPON_01].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x4;
	}
	if(lpObj->pInventory[WEAPON_02].IsExtItem())
	{
		lpObj->CharSet[10] |= 0x2;
	}*/
	this->charset[11] = 0;
	/*if(lpObj->pInventory[HELMET].IsSetItem())
	{
		lpObj->CharSet[11] = 0x80;
	}
	if(lpObj->pInventory[ARMOR].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x40;
	}
	if(lpObj->pInventory[PANTS].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x20;
	}
	if(lpObj->pInventory[GLOVES].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x10;
	}
	if(lpObj->pInventory[BOOTS].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x8;
	}
	if(lpObj->pInventory[WEAPON_01].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x4;
	}
	if(lpObj->pInventory[WEAPON_02].IsSetItem())
	{
		lpObj->CharSet[11] |= 0x2;
	}
	if(lpObj->IsFullSetItem)
	{
		lpObj->CharSet[11] |= 0x01;
	}*/
}