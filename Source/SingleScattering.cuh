/*
	Copyright (c) 2011, T. Kroes <t.kroes@tudelft.nl>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	- Neither the name of the TU Delft nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "Transport.cuh"

KERNEL void KrnlSingleScattering(FrameBuffer* pFrameBuffer)
{
	const int X		= blockIdx.x * blockDim.x + threadIdx.x;
	const int Y		= blockIdx.y * blockDim.y + threadIdx.y;

	if (X >= pFrameBuffer->m_Resolution[0] || Y >= pFrameBuffer->m_Resolution[1])
		return;
	
	CRNG RNG(pFrameBuffer->m_RandomSeeds1.GetPtr(X, Y), pFrameBuffer->m_RandomSeeds2.GetPtr(X, Y));

	Vec2f ScreenPoint;

	ScreenPoint.x = gCamera.m_Screen[0][0] + (gCamera.m_InvScreen[0] * (float)X);
	ScreenPoint.y = gCamera.m_Screen[1][0] + (gCamera.m_InvScreen[1] * (float)Y);
	
	CRay Re;

	Re.m_O		= ToVec3f(gCamera.m_Pos);
	Re.m_D		= Normalize(ToVec3f(gCamera.m_N) + (ScreenPoint.x * ToVec3f(gCamera.m_U)) - (ScreenPoint.y * ToVec3f(gCamera.m_V)));
	Re.m_MinT	= gCamera.m_ClipNear;
	Re.m_MaxT	= gCamera.m_ClipFar;

	if (gCamera.m_ApertureSize != 0.0f)
	{
		const Vec2f LensUV = gCamera.m_ApertureSize * ConcentricSampleDisk(RNG.Get2());

		const Vec3f LI = ToVec3f(gCamera.m_U) * LensUV.x + ToVec3f(gCamera.m_V) * LensUV.y;

		Re.m_O += LI;
		Re.m_D = Normalize(Re.m_D * gCamera.m_FocalDistance - LI);
	}

	ColorXYZf Lv = SPEC_BLACK, Li = SPEC_BLACK;

	Vec3f Pe, Pl;
	
	float T = 0.0f;

	if (SampleDistanceRM(Re, RNG, Pe, T) && T >= gCamera.m_ClipNear && T < gCamera.m_ClipFar)
	{
		if (NearestLight(CRay(Re.m_O, Re.m_D, 0.0f, (Pe - Re.m_O).Length()), Li, Pl, true))
		{
			pFrameBuffer->m_FrameEstimateXyza.Set(ColorXYZAf(Li), X, Y);
			return;
		}

		const float Intensity = GetIntensity(Pe);

		Lv += GetEmission(Intensity);

		switch (gVolume.m_ShadingType)
		{
			case 0:
			{
				Lv += UniformSampleOneLight(CVolumeShader::Brdf, Intensity, Normalize(-Re.m_D), Pe, NormalizedGradient(Pe), RNG);
				break;
			}
		
			case 1:
			{
				Lv += UniformSampleOneLight(CVolumeShader::Phase, Intensity, Normalize(-Re.m_D), Pe, NormalizedGradient(Pe), RNG);
				break;
			}
			
			/*
			case 2:
			{
				const float GradMag = GradientMagnitude(Pe) * gVolume.m_IntensityInvRange;
				const float PdfBrdf = (1.0f - __expf(-pScene->m_GradientFactor * GradMag));

				if (RNG.Get1() < PdfBrdf)
  					Lv += UniformSampleOneLight(pScene, CVolumeShader::Brdf, D, Normalize(-Re.m_D), Pe, NormalizedGradient(Pe), RNG, true);
				else
					Lv += 0.5f * UniformSampleOneLight(pScene, CVolumeShader::Phase, D, Normalize(-Re.m_D), Pe, NormalizedGradient(Pe), RNG, false);

				break;
			}
			*/
		}
	}
	else
	{
		/*
		float NearestT = 1500.0f;

		for (int i = 0; i < gReflections.m_NoReflectionObjects; i++)
		{
			CRay TR = TransformRay(Re, gReflections.m_ReflectionObjects[i].m_InvTM);

			int Res = 0;

			float T = 0.0f;

			Res = IntersectPlane(TR, false, Vec3f(1.0f), &T, NULL);
			
			if (Res > 0)
			{
				if (T < NearestT)
				{
					NearestT = T;
				}
			}

			if (NearestT < 1500.0f)
			{
				Lv = ColorXYZf(1.0f);


				Vec3f HitP = Re(NearestT);

				CVolumeShader Shader(CVolumeShader::Brdf, Vec3f(0.0f, 1.0f, 0.0f), -Re.m_D, ColorXYZf(1.0f), ColorXYZf(1.0f), 5.0f, 50.0f);

				Vec3f Wi, Pl;

				float LightPdf = 1.0f, ShaderPdf = 1.0f;

				ColorXYZf F = Shader.SampleF(Wo, Wi, ShaderPdf, LS.m_BsdfSample);



			}
			else
			{
				Lv = ColorXYZf(0.0f);
			}
		}
		*/
		
		/*
		if (NearestLight(CRay(Re.m_O, Re.m_D, 0.0f, (Pe - Re.m_O).Length()), Li, Pl, true))
		{
			pFrameBuffer->m_FrameEstimateXyza.Set(ColorXYZAf(Li), X, Y);
			return;
		}


		if (NearestLight(CRay(Re.m_O, Re.m_D, 0.0f, INF_MAX), Li, Pl, true))
			Lv = Li;
		*/
	}

	ColorXYZAf L(Lv.GetX(), Lv.GetY(), Lv.GetZ(), 0.0f);

	pFrameBuffer->m_FrameEstimateXyza.Set(L, X, Y);
}

void SingleScattering(FrameBuffer* pFrameBuffer, int Width, int Height)
{
	const dim3 BlockDim(KRNL_SINGLE_SCATTERING_BLOCK_W, KRNL_SINGLE_SCATTERING_BLOCK_H);
	const dim3 GridDim((int)ceilf((float)Width / (float)BlockDim.x), (int)ceilf((float)Height / (float)BlockDim.y));

	KrnlSingleScattering<<<GridDim, BlockDim>>>(pFrameBuffer);
	cudaThreadSynchronize();
	HandleCudaKernelError(cudaGetLastError(), "Single Scattering");
}