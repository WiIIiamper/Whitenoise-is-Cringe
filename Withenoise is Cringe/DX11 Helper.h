#pragma once
#include <Windows.h>
#include "d3d11.h"
#include <mupdf/fitz.h>
#include "imgui_template.h"

class DX11Helper {
public:
	static void mupdfPixmapToShaderResourceView(fz_pixmap* pixmap, ID3D11ShaderResourceView** outSRV) {
		int pixmapWidth = pixmap->w;
		int pixmapHeight = pixmap->h;

        auto textureDesc = Create2DTextureDescription(pixmapWidth, pixmapHeight);
        ID3D11Texture2D* Texture2D = NULL;
        auto subResource = CreateSubResourceFromMuPDFPixmap(pixmap);

        ID3D11Texture2D* pTexture = NULL;
        g_pd3dDevice->CreateTexture2D(&textureDesc, &subResource, &pTexture);

        CreateShaderResourceView(textureDesc, pTexture, outSRV);
	}

	static D3D11_TEXTURE2D_DESC Create2DTextureDescription(int width, int height) {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        return desc;
	}

    static D3D11_SUBRESOURCE_DATA CreateSubResourceFromMuPDFPixmap(fz_pixmap* pixmap) {
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = pixmap->samples;
        subResource.SysMemPitch = pixmap->stride;
        subResource.SysMemSlicePitch = 0;
        return subResource;
    }

    static void CreateShaderResourceView
    (D3D11_TEXTURE2D_DESC desc, ID3D11Texture2D* pTexture, ID3D11ShaderResourceView** outSRV) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, outSRV);
        pTexture->Release();
    }
};