
# TitaniumRHI

TitaniumRHI is a low-level Rendering Hardware Interface (RHI) that provides an abstraction over multiple graphics APIs.

## Resource Lifetime Management

**The user is responsible for managing the lifetime of RHI resources.**

All resources (textures, buffers, samplers, descriptor sets, pipelines, etc.) must remain valid until every command list referencing them has been submitted or discarded.

### Example: Invalid Resource Lifetime

```cpp
{
    Texture texture(device);

    commandList.bindTexture(texture);

} // Texture is destroyed here.

// INVALID: The command list still references the destroyed texture.
device.submit(commandList);
```

In this example, `Texture` is destroyed before the command list is submitted. This results in undefined behavior because the command list internally tracks a raw pointer to the resource.

### Internal Resource Tracking

TitaniumRHI implements backend-specific resource tracking to safely manage native GPU handle destruction.

For example, the Vulkan backend uses **timeline semaphores** to track the last GPU submission that references a resource.

When a command list is submitted:

1. The backend assigns a timeline semaphore value to the submission.
2. All resources referenced by the command list are marked with that submission value.
3. When a resource is destroyed, its native GPU handles are scheduled for deferred destruction.
4. The backend releases those handles once the GPU has completed the relevant submissions.

This mechanism prevents native GPU resources from being destroyed while they are still in use by the GPU.

### Important

**Internal resource tracking does not extend the lifetime of C++ resource objects.**

Users must ensure that:

- Resources remain alive until all referencing command lists have been submitted or discarded.
- Resources are not used after destruction.
- The RHI device remains alive until all associated resources and pending GPU operations have been cleaned up.

Once a command list has been successfully submitted and its resources have been tracked, resource wrappers may be destroyed without explicitly waiting for GPU completion. TitaniumRHI handles the deferred destruction of their native GPU handles.
