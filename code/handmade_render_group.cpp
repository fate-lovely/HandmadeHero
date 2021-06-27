inline v4
Unpack4x8(u32 color)
{
  v4 result = { (f32)((color >> 16) & 0xff),
    (f32)((color >> 8) & 0xff),
    (f32)((color >> 0) & 0xff),
    (f32)((color >> 24) & 0xff) };
  return result;
}

inline u32
Pack4x8(v4 value)
{
  u32 result = ((u32)(value.a + 0.5f) << 24) | ((u32)(value.r + 0.5f) << 16)
    | ((u32)(value.g + 0.5f) << 8) | (u32)(value.b + 0.5f);

  return result;
}

inline u32
SRGB1ToUint32(v4 color)
{
  u32 result = Pack4x8(color * 255.0f);

  return result;
}

inline v4
UnscaleAndBiasNormal(v4 normal)
{
  v4 result;

  f32 inv255 = 1.0f / 255.0f;

  result.x = -1.0f + 2.0f * (normal.x * inv255);
  result.y = -1.0f + 2.0f * (normal.y * inv255);
  result.z = -1.0f + 2.0f * (normal.z * inv255);

  result.w = normal.w * inv255;

  return result;
}

inline v4
SRGB255ToLinear1(v4 color)
{
  v4 result = {};
  f32 inv255 = 1.0f / 255.0f;
  result.r = Square(color.r * inv255);
  result.g = Square(color.g * inv255);
  result.b = Square(color.b * inv255);
  result.a = color.a * inv255;
  return result;
}

inline v4
Linear1ToSRGB255(v4 color)
{
  v4 result = {};
  result.r = 255.0f * SquareRoot(color.r);
  result.g = 255.0f * SquareRoot(color.g);
  result.b = 255.0f * SquareRoot(color.b);
  result.a = 255.0f * color.a;
  return result;
}

internal void
DrawBitmap(loaded_bitmap *buffer,
  loaded_bitmap *bitmap,
  v2 minCorner,
  f32 cAlpha = 1.0f)
{
  i32 minX = RoundReal32ToInt32(minCorner.x);
  i32 minY = RoundReal32ToInt32(minCorner.y);
  i32 maxX = minX + bitmap->width;
  i32 maxY = minY + bitmap->height;

  i32 clipX = 0;
  if(minX < 0) {
    clipX = -minX;
    minX = 0;
  }
  i32 clipY = 0;
  if(minY < 0) {
    clipY = -minY;
    minY = 0;
  }
  if(maxX > buffer->width) {
    maxX = buffer->width;
  }
  if(maxY > buffer->height) {
    maxY = buffer->height;
  }

  u8 *sourceRow
    = (u8 *)bitmap->memory + clipY * bitmap->pitch + clipX * BYTES_PER_PIXEL;
  u8 *destRow
    = (u8 *)buffer->memory + minY * buffer->pitch + minX * BYTES_PER_PIXEL;

  for(int y = minY; y < maxY; y++) {
    u32 *source = (u32 *)sourceRow;
    u32 *dest = (u32 *)destRow;

    for(int x = minX; x < maxX; x++) {
      v4 texel = {
        (f32)((*source >> 16) & 0xff),
        (f32)((*source >> 8) & 0xff),
        (f32)((*source >> 0) & 0xff),
        (f32)((*source >> 24) & 0xff),
      };

      texel = SRGB255ToLinear1(texel);

      texel *= cAlpha;

      v4 d = {
        (f32)((*dest >> 16) & 0xff),
        (f32)((*dest >> 8) & 0xff),
        (f32)((*dest >> 0) & 0xff),
        (f32)((*dest >> 24) & 0xff),
      };
      d = SRGB255ToLinear1(d);

      v4 blended = (1.0f - texel.a) * d + texel;
      blended = Linear1ToSRGB255(blended);

      // clang-format off
      *dest = ((u32)(blended.a + 0.5f) << 24) |
        ((u32)(blended.r + 0.5f) << 16) |
        ((u32)(blended.g + 0.5f) << 8) |
        ((u32)(blended.b + 0.5f));
      // clang-format on

      dest++;
      source++;
    }

    sourceRow += bitmap->pitch;
    destRow += buffer->pitch;
  }
}

internal void
DrawMatte(loaded_bitmap *buffer,
  loaded_bitmap *bitmap,
  v2 minCorner,
  f32 cAlpha = 1.0f)
{
  i32 minX = RoundReal32ToInt32(minCorner.x);
  i32 minY = RoundReal32ToInt32(minCorner.y);
  i32 maxX = minX + bitmap->width;
  i32 maxY = minY + bitmap->height;

  i32 clipX = 0;
  if(minX < 0) {
    clipX = -minX;
    minX = 0;
  }
  i32 clipY = 0;
  if(minY < 0) {
    clipY = -minY;
    minY = 0;
  }
  if(maxX > buffer->width) {
    maxX = buffer->width;
  }
  if(maxY > buffer->height) {
    maxY = buffer->height;
  }

  u8 *sourceRow
    = (u8 *)bitmap->memory + clipY * bitmap->pitch + clipX * BYTES_PER_PIXEL;
  u8 *destRow
    = (u8 *)buffer->memory + minY * buffer->pitch + minX * BYTES_PER_PIXEL;

  for(int y = minY; y < maxY; y++) {
    u32 *source = (u32 *)sourceRow;
    u32 *dest = (u32 *)destRow;

    for(int x = minX; x < maxX; x++) {
      v4 texel = {
        (f32)((*source >> 16) & 0xff),
        (f32)((*source >> 8) & 0xff),
        (f32)((*source >> 0) & 0xff),
        (f32)((*source >> 24) & 0xff),
      };

      texel = SRGB255ToLinear1(texel);
      texel *= cAlpha;

      v4 d = {
        (f32)((*dest >> 16) & 0xff),
        (f32)((*dest >> 8) & 0xff),
        (f32)((*dest >> 0) & 0xff),
        (f32)((*dest >> 24) & 0xff),
      };

      v4 blended = (1.0f - texel.a) * d;

      blended = Linear1ToSRGB255(blended);

      // clang-format off
      *dest = ((u32)(blended.a + 0.5f) << 24) |
        ((u32)(blended.r + 0.5f) << 16) |
        ((u32)(blended.g + 0.5f) << 8) |
        ((u32)(blended.b + 0.5f));
      // clang-format on

      dest++;
      source++;
    }

    sourceRow += bitmap->pitch;
    destRow += buffer->pitch;
  }
}

struct bilinear_sample {
  u32 a, b, c, d;
};

inline bilinear_sample
BilinearSample(loaded_bitmap *bitmap, i32 x, i32 y)
{
  Assert(x >= 0 && x < bitmap->width - 1);
  Assert(y >= 0 && y < bitmap->height - 1);

  bilinear_sample result;
  u8 *ptr = (u8 *)(bitmap->memory) + y * bitmap->pitch + x * 4;

  result.a = *((u32 *)ptr);
  result.b = *((u32 *)(ptr + 4));
  result.c = *((u32 *)(ptr + bitmap->pitch));
  result.d = *((u32 *)(ptr + bitmap->pitch + 4));

  return result;
}

v4
SRGBBilinearBlend(bilinear_sample sample, f32 fx, f32 fy)
{
  v4 texelA = Unpack4x8(sample.a);
  v4 texelB = Unpack4x8(sample.b);
  v4 texelC = Unpack4x8(sample.c);
  v4 texelD = Unpack4x8(sample.d);

  texelA = SRGB255ToLinear1(texelA);
  texelB = SRGB255ToLinear1(texelB);
  texelC = SRGB255ToLinear1(texelC);
  texelD = SRGB255ToLinear1(texelD);

  v4 result = Lerp(Lerp(texelA, fx, texelB), fy, Lerp(texelC, fx, texelD));

  return result;
}

inline v3
SampleEnvironmentMap(v2 screenSpaceUV,
  v3 sampleDirection,
  f32 roughness,
  environment_map *map,
  f32 distanceFromMapInZ)
{
  /*
    NOTE(cj):

    screenSpaceUV tells us where the ray is being cast _from_ in normalized
    screen coordinates.

    sampleDirection tells us what direction the cast is going - it does not have
    to be normalized.

    roughness says which LODs of map we sample from.

    distanceFromMapInZ says how far the map is from the sample point in Z, given
    in meters.
  */

  // NOTE(cj): Pick which LOD to sample from
  u32 lodIndex = (u32)(roughness * (f32)(ArrayCount(map->lod) - 1) + 0.5f);
  Assert(lodIndex < ArrayCount(map->lod));

  loaded_bitmap *lod = map->lod + lodIndex;

  f32 uvsPerMeter = 0.1f;
  f32 C = distanceFromMapInZ / sampleDirection.y;
  v2 offset = C * V2(sampleDirection.x, sampleDirection.z);
  v2 uv = screenSpaceUV + uvsPerMeter * offset;

  uv = Clamp01(uv);

  f32 tx = uv.x * (f32)(lod->width - 2);
  f32 ty = uv.y * (f32)(lod->height - 2);

  i32 x = (i32)tx;
  i32 y = (i32)ty;

  f32 fx = tx - (f32)x;
  f32 fy = ty - (f32)y;

  Assert(x >= 0 && x < lod->width - 1);
  Assert(y >= 0 && y < lod->height - 1);

#if 0
  u8 *texelPtr = (u8 *)lod->memory + y * lod->pitch + x * 4;
  *(u32 *)texelPtr = 0xFFFFFFFF;
#endif

  bilinear_sample sample = BilinearSample(lod, x, y);
  v3 result = SRGBBilinearBlend(sample, fx, fy).xyz;

  return result;
}

// exclusive
internal void
DrawRectangleSlowly(loaded_bitmap *buffer,
  v2 origin,
  v2 xAxis,
  v2 yAxis,
  v4 color,
  loaded_bitmap *texture,
  loaded_bitmap *normalMap,
  environment_map *top,
  environment_map *middle,
  environment_map *bottom,
  f32 pixelsToMeters)
{
  color.rgb *= color.a;

  u32 c = SRGB1ToUint32(color);

  f32 xAxisLength = Length(xAxis);
  f32 yAxisLength = Length(yAxis);

  v2 nXAxis = (yAxisLength / xAxisLength) * xAxis;
  v2 nYAxis = (xAxisLength / yAxisLength) * yAxis;

  // NOTE(cj): this could be a parameter if we want people to
  // have control over the amount of scaling in the Z direction.
  f32 nZScale = 0.5f * (xAxisLength + yAxisLength);

  f32 invXAxisLengthSq = 1.0f / LengthSq(xAxis);
  f32 invYAxisLengthSq = 1.0f / LengthSq(yAxis);

  i32 maxWidth = buffer->width - 1;
  i32 maxHeight = buffer->height - 1;

  f32 invMaxWidth = 1.0f / (f32)maxWidth;
  f32 invMaxHeight = 1.0f / (f32)maxHeight;

  i32 minX = maxWidth;
  i32 minY = maxHeight;
  i32 maxX = 0;
  i32 maxY = 0;

  f32 originZ = 0.0f;
  f32 originY = (origin + 0.5f * xAxis + 0.5f * yAxis).y;
  f32 fixedCastY = invMaxHeight * originY;

  v2 ps[4] = { origin, origin + xAxis, origin + yAxis, origin + xAxis + yAxis };
  for(int pIndex = 0; pIndex < ArrayCount(ps); pIndex++) {
    v2 p = ps[pIndex];
    i32 floorX = FloorReal32ToInt32(p.x);
    i32 floorY = FloorReal32ToInt32(p.y);

    if(minX > floorX) {
      minX = floorX;
    }
    if(maxX < floorX) {
      maxX = floorX;
    }
    if(minY > floorY) {
      minY = floorY;
    }
    if(maxY < floorY) {
      maxY = floorY;
    }
  }

  if(minX < 0) {
    minX = 0;
  }
  if(minY < 0) {
    minY = 0;
  }
  if(maxX > maxWidth) {
    maxX = maxWidth;
  }
  if(maxY > maxHeight) {
    maxY = maxHeight;
  }

  u8 *row = (u8 *)buffer->memory + minY * buffer->pitch + minX * 4;

  for(int y = minY; y <= maxY; y++) {
    u32 *pixel = (u32 *)row;

    for(int x = minX; x <= maxX; x++) {
      v2 p = V2(x, y);
      v2 d = p - origin;

      f32 edgeTop = Inner(Perp(xAxis), d - yAxis);
      f32 edgeBottom = Inner(-Perp(xAxis), d);
      f32 edgeLeft = Inner(Perp(yAxis), d);
      f32 edgeRight = Inner(-Perp(yAxis), d - xAxis);

      if((edgeTop < 0) && (edgeBottom < 0) && (edgeLeft < 0)
        && (edgeRight < 0)) {
        v2 screenSpaceUV = { invMaxWidth * (f32)x, fixedCastY };
        f32 zDiff = pixelsToMeters * ((f32)y - originY);

        // NOTE(cj): xAxis must be perpendicular with yAxis
        f32 u = Inner(d, xAxis) * invXAxisLengthSq;
        f32 v = Inner(d, yAxis) * invYAxisLengthSq;

        // f32 epsilon = 1.0f;
        Assert(u >= 0.0f && u <= 1.0f);
        Assert(v >= 0.0f && v <= 1.0f);

        // TODO: SSE clamping
        f32 tPx = u * (f32)(texture->width - 2);
        f32 tPy = v * (f32)(texture->height - 2);

        i32 tx = (i32)tPx;
        i32 ty = (i32)tPy;

        f32 fx = tPx - (f32)tx;
        f32 fy = tPy - (f32)ty;

        bilinear_sample bilinearSample = BilinearSample(texture, tx, ty);
        v4 texel = SRGBBilinearBlend(bilinearSample, fx, fy);

        if(normalMap) {
          bilinear_sample normalSample = BilinearSample(normalMap, tx, ty);
          v4 normalA = Unpack4x8(normalSample.a);
          v4 normalB = Unpack4x8(normalSample.b);
          v4 normalC = Unpack4x8(normalSample.c);
          v4 normalD = Unpack4x8(normalSample.d);

          v4 normal
            = Lerp(Lerp(normalA, fx, normalB), fy, Lerp(normalC, fx, normalD));

          normal = UnscaleAndBiasNormal(normal);

          // Rotate the normal
          normal.xy = normal.x * nXAxis + normal.y * nYAxis;
          normal.z *= nZScale;
          normal.xyz = Normalize(normal.xyz);

          // NOTE(cj): The eye vector is always assumed to be [0, 0, 1]
          // This is the simplified version of -e + 2Inner(e, N)N
          v3 bouceDirection = 2.0f * normal.z * normal.xyz - V3(0, 0, 1);
          bouceDirection.z = -bouceDirection.z;

          environment_map *farMap = 0;
          f32 z = originZ + zDiff;
          f32 mapZ = 2.0f;
          f32 tEnvMap = bouceDirection.y;
          f32 tFarMap = 0.0f;

          if(tEnvMap < -0.5f) {
            farMap = bottom;
            tFarMap = 2.0f * (-tEnvMap - 0.5f);
          } else if(tEnvMap > 0.5f) {
            farMap = top;
            tFarMap = 2.0f * (tEnvMap - 0.5f);
          }

          tFarMap *= tFarMap;
          tFarMap *= tFarMap;

          v3 lightColor = { 0, 0, 0 };
          if(farMap) {
            f32 distanceFromMapInZ = farMap->z - z;
            v3 farMapColor = SampleEnvironmentMap(screenSpaceUV,
              bouceDirection,
              normal.w,
              farMap,
              distanceFromMapInZ);
            lightColor = Lerp(lightColor, tFarMap, farMapColor);
          }

          texel.rgb = texel.rgb + texel.a * lightColor;
          // texel.rgb = V3(0.5f, 0.5f, 0.5f) + 0.5f * bouceDirection;
        }

        texel = Hadamard(texel, color);
        texel.r = Clamp01(texel.r);
        texel.g = Clamp01(texel.g);
        texel.b = Clamp01(texel.b);

        v4 dest = SRGB255ToLinear1(Unpack4x8(*pixel));

        // texel is premultiplied alpha
        v4 blended = (1.0f - texel.a) * dest + texel;

        blended = Linear1ToSRGB255(blended);
        *pixel = Pack4x8(blended);
      }

      pixel++;
    }

    row += buffer->pitch;
  }
}

// exclusive: [min, max)
internal void
DrawRectangle(loaded_bitmap *buffer, v2 min, v2 max, v4 color)
{
  i32 minX = RoundReal32ToInt32(min.x);
  i32 minY = RoundReal32ToInt32(min.y);
  i32 maxX = RoundReal32ToInt32(max.x);
  i32 maxY = RoundReal32ToInt32(max.y);

  if(minX < 0) {
    minX = 0;
  }
  if(minY < 0) {
    minY = 0;
  }
  if(maxX > buffer->width) {
    maxX = buffer->width;
  }
  if(maxY > buffer->height) {
    maxY = buffer->height;
  }

  u32 c = SRGB1ToUint32(color);

  u8 *row
    = (u8 *)buffer->memory + minY * buffer->pitch + minX * BYTES_PER_PIXEL;

  for(int y = minY; y < maxY; y++) {
    u32 *Pixel = (u32 *)row;
    for(int x = minX; x < maxX; x++) {
      *Pixel++ = c;
    }

    row += buffer->pitch;
  }
}

internal void
DrawRectangleOutline(loaded_bitmap *buffer, v2 min, v2 max, v4 color)
{
  f32 r = 2;

  // top and bottom
  DrawRectangle(buffer,
    V2(min.x - r, max.y - r),
    V2(max.x + r, max.y + r),
    color);
  DrawRectangle(buffer,
    V2(min.x - r, min.y - r),
    V2(max.x + r, min.y + r),
    color);

  // left and right
  DrawRectangle(buffer,
    V2(min.x - r, min.y - r),
    V2(min.x + r, max.y + r),
    color);
  DrawRectangle(buffer,
    V2(max.x - r, min.y - r),
    V2(max.x + r, max.y + r),
    color);
}

internal render_group *
AllocateRenderGroup(memory_arena *arena,
  u32 maxPushBufferSize,
  f32 metersToPixels)
{
  render_group *result = PushStruct(arena, render_group);
  render_basis *defaultBasis = PushStruct(arena, render_basis);
  defaultBasis->p = {};

  result->defaultBasis = defaultBasis;
  result->pieceCount = 0;
  result->metersToPixels = metersToPixels;

  result->maxPushBufferSize = maxPushBufferSize;
  result->pushBufferSize = 0;
  result->pushBufferBase = (u8 *)PushSize(arena, maxPushBufferSize);

  return result;
}

#define PushRenderElement(group, type)                                         \
  (type *)_PushRenderElement(group, sizeof(type), RenderEntryType_##type)

internal void *
_PushRenderElement(render_group *group, u32 size, render_entry_type type)
{
  size += sizeof(render_entry_header);
  Assert(group->pushBufferSize + size <= group->maxPushBufferSize);

  render_entry_header *header
    = (render_entry_header *)(group->pushBufferBase + group->pushBufferSize);
  group->pushBufferSize += size;

  header->type = type;

  return header + 1;
}

// `offset` is from the min corner
internal void
PushBitmap(render_group *group,
  loaded_bitmap *bitmap,
  v3 offset,
  v4 color = V4(1, 1, 1, 1))
{
  render_entry_bitmap *entry = PushRenderElement(group, render_entry_bitmap);
  entry->entityBasis.offset
    = group->metersToPixels * offset - V3(bitmap->align, 0.0f);
  entry->entityBasis.basis = group->defaultBasis;
  entry->bitmap = bitmap;
  entry->color = color;
}

internal void
Clear(render_group *group, v4 color)
{
  render_entry_clear *entry = PushRenderElement(group, render_entry_clear);
  entry->color = color;
}

internal render_entry_coordinate_system *
CoordinateSystem(render_group *group,
  v2 origin,
  v2 xAxis,
  v2 yAxis,
  v4 color,
  loaded_bitmap *texture,
  loaded_bitmap *normalMap,
  environment_map *top,
  environment_map *middle,
  environment_map *bottom)
{
  render_entry_coordinate_system *entry
    = PushRenderElement(group, render_entry_coordinate_system);

  entry->origin = origin;
  entry->xAxis = xAxis;
  entry->yAxis = yAxis;
  entry->color = color;
  entry->texture = texture;
  entry->normalMap = normalMap;
  entry->top = top;
  entry->middle = middle;
  entry->bottom = bottom;

  return entry;
}

// `offset` is from the center
internal void
PushRect(render_group *group, v3 offset, v2 dim, v4 color = V4(1, 1, 1, 1))
{
  render_entry_rectangle *entry
    = PushRenderElement(group, render_entry_rectangle);
  entry->entityBasis.offset
    = group->metersToPixels * (offset - V3(0.5f * dim, 0.0f));
  entry->entityBasis.basis = group->defaultBasis;
  entry->color = color;
  entry->dim = dim * group->metersToPixels;
}

// `offset` is from the center
internal void
PushRectOutline(render_group *group,
  v3 offset,
  v2 dim,
  v4 color = V4(1, 1, 1, 1))
{
  f32 thickness = 0.1f;
  // Top and bottom
  PushRect(group, offset - V3(0, 0.5f * dim.y, 0), V2(dim.x, thickness), color);
  PushRect(group, offset + V3(0, 0.5f * dim.y, 0), V2(dim.x, thickness), color);

  // Left and right
  PushRect(group, offset - V3(0, 0.5f * dim.x, 0), V2(thickness, dim.y), color);
  PushRect(group, offset + V3(0, 0.5f * dim.x, 0), V2(thickness, dim.y), color);
}

struct render_basis_result {
  f32 scale;
  v2 p;
};

internal render_basis_result
GetEntityRenderBasisResult(render_entity_basis *entityBasis,
  v2 screenCenter,
  f32 metersToPixels)
{
  render_basis_result result;

  v3 entityP = metersToPixels * entityBasis->basis->p;
  f32 zFudge = 1.0f + 0.001f * entityP.z;
  v2 min = screenCenter + zFudge * (entityP.xy + entityBasis->offset.xy);
  result.p = min; // V2(0.0f, entityP.z + entityBasis->offset.z);
  result.scale = zFudge;

  return result;
}

internal void
RenderGroupToOutput(render_group *renderGroup, loaded_bitmap *outputTarget)
{
  f32 metersToPixels = renderGroup->metersToPixels;
  v2 screenCenter
    = 0.5f * v2{ (f32)outputTarget->width, (f32)outputTarget->height };

  f32 pixelsToMeters = 1.0f / renderGroup->metersToPixels;

  for(u32 offset = 0; offset < renderGroup->pushBufferSize;) {
    render_entry_header *header
      = (render_entry_header *)(renderGroup->pushBufferBase + offset);
    offset += sizeof(render_entry_header);
    void *data = (void *)(header + 1);

    switch(header->type) {
      case RenderEntryType_render_entry_clear: {
        render_entry_clear *entry = (render_entry_clear *)data;
        offset += sizeof(render_entry_clear);

        DrawRectangle(outputTarget,
          V2(0, 0),
          V2((f32)outputTarget->width, (f32)outputTarget->height),
          entry->color);
      } break;

      case RenderEntryType_render_entry_rectangle: {
        render_entry_rectangle *entry = (render_entry_rectangle *)data;
        offset += sizeof(render_entry_rectangle);
        v3 entityP = entry->entityBasis.basis->p;

        render_basis_result basisResult
          = GetEntityRenderBasisResult(&entry->entityBasis,
            screenCenter,
            renderGroup->metersToPixels);

        DrawRectangle(outputTarget,
          basisResult.p,
          basisResult.p + basisResult.scale * entry->dim,
          entry->color);
      } break;

      case RenderEntryType_render_entry_bitmap: {
        render_entry_bitmap *entry = (render_entry_bitmap *)data;
        offset += sizeof(render_entry_bitmap);

        render_basis_result basisResult
          = GetEntityRenderBasisResult(&entry->entityBasis,
            screenCenter,
            renderGroup->metersToPixels);

        // DrawBitmap(outputTarget, entry->bitmap, min, entry->color.a);
        DrawRectangleSlowly(outputTarget,
          basisResult.p,
          basisResult.scale * V2(entry->bitmap->width, 0),
          basisResult.scale * V2(0, entry->bitmap->height),
          entry->color,
          entry->bitmap,
          0,
          0,
          0,
          0,
          pixelsToMeters);

      } break;

      case RenderEntryType_render_entry_coordinate_system: {
        render_entry_coordinate_system *entry
          = (render_entry_coordinate_system *)data;
        offset += sizeof(render_entry_coordinate_system);

        v2 min = entry->origin;
        v2 max = entry->origin + entry->xAxis + entry->yAxis;
        DrawRectangleSlowly(outputTarget,
          entry->origin,
          entry->xAxis,
          entry->yAxis,
          entry->color,
          entry->texture,
          entry->normalMap,
          entry->top,
          entry->middle,
          entry->bottom,
          pixelsToMeters);

        v2 dim = V2(4, 4);
        v2 p = entry->origin;
        v4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
        DrawRectangle(outputTarget, p, p + dim, color);

        p = entry->origin + entry->xAxis;
        DrawRectangle(outputTarget, p, p + dim, color);

        p = entry->origin + entry->yAxis;
        DrawRectangle(outputTarget, p, p + dim, color);

        DrawRectangle(outputTarget, max, max + dim, color);
#if 0
        for(u32 i = 0; i < ArrayCount(entry->points); i++) {
          v2 p = entry->points[i];
          v2 min = entry->origin + p.x * entry->xAxis + p.y * entry->yAxis;
          DrawRectangle(outputTarget, min, min + dim, entry->color);
        }
#endif
      } break;

        InvalidDefaultCase;
    }
  }
}
