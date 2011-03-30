/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/

#ifndef TRANSFER_FUNCTION_H_
#define TRANSFER_FUNCTION_H_

class CTransferFunc
{
public:
    CTransferFunc();
    virtual ~CTransferFunc();

    void checkError();

    void init_TF(const char*);
    void save_TF(const char*);


    void init_colormap(float, float, float);
    void init_alphamap(double scale,
                       double thres, 
                       double shift,
                       double intensity,                            
                       int numPeaks);
    void update_TF(int, int, int, int, int);

    unsigned char* get_TF(){return m_TF;}
    int get_TFsize(){return m_TFsize;}

    void draw_preInt_texture(int, int);
    void draw_transfer_function(int, int, int m, int c);
    void draw_alphamap(int, int);
    void draw_colormap(int, int);

    void init_TF_texture();
    void init_preInt_texture();
    void update_textures();
    tex_unit* get_TF_texture() {return m_transferFunc;}
    tex_unit* get_preInt_texture() { return m_preInt; }

private:
    int m_TFsize;
    unsigned char* m_TF;

    tex_unit *m_transferFunc;
    tex_unit *m_preInt;

    unsigned char* create_Preintegration_Table( unsigned char* Table );
    void draw_transfer_function_component(int c);
};

#endif
