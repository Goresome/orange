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

#ifndef _CSHOP_H_
#define _CSHOP_H_

#include ".\\Item.h"

class CShop
{
private:
  /*<thisrel this+0x4>*/ /*|0x78|*/ unsigned char ShopInventoryMap[120];
  /*<thisrel this+0x7c>*/ /*|0x4|*/ int ItemCount;
  /*<thisrel this+0x80>*/ /*|0x5280|*/ CItem m_item[120];
  /*<thisrel this+0x5300>*/ /*|0x3c0|*/ unsigned char SendItemData[960];
  /*<thisrel this+0x56c0>*/ /*|0x4|*/ int SendItemDataLen;
public:
  CShop();
  void Init();
  int LoadShopItem(/*<regrel ebp+0x8>*/ /*|0x4|*/ int Shopnumber);
  int LoadShopItem(/*<regrel ebp+0x8>*/ /*|0x4|*/ char* filename);
  int InsertItem(/*<regrel ebp+0x8>*/ /*|0x4|*/ int type, /*<regrel ebp+0xc>*/ /*|0x4|*/ int index, /*<regrel ebp+0x10>*/ /*|0x4|*/ int level, /*<regrel ebp+0x14>*/ /*|0x4|*/ int dur, /*<regrel ebp+0x18>*/ /*|0x4|*/ int op1, /*<regrel ebp+0x1c>*/ /*|0x4|*/ int op2, /*<regrel ebp+0x20>*/ /*|0x4|*/ int op3);
  int InentoryMapCheck(/*<regrel ebp+0x8>*/ /*|0x4|*/ int sx, /*<regrel ebp+0xc>*/ /*|0x4|*/ int sy, /*<regrel ebp+0x10>*/ /*|0x4|*/ int width, /*<regrel ebp+0x14>*/ /*|0x4|*/ int height);
  virtual /*<vtableoff 0x0>*/ ~CShop();
};
// <size 0x56c4>

#endif