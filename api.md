## Lua API

This is no longer updated, see [ecs.lua](ecs.lua).

#### `ecs.new([id], [[name : string], components : string])`

Create a new entity with a specific `id` (optional),
if `name` is given a named entity will be created.
The entity can be optionally initialized with a
set of `components`.

Returns the entity id.

#### `ecs.bulk_new([type], n)`

Create `n` new entities with components.

Returns an array of entities

#### `ecs.delete(entity)`

Delete an entity and all of its components.

`entity` should be a name or id.

#### `ecs.tag(name)`

Create a tag.

Returns the tag entity.

#### `ecs.name(id)`

Get the name of an entity.

This will return the name as specified in the `EcsName` component.

#### `ecs.lookup(name)`

Look up an entity by name

#### `ecs.has(id, type)`

Test if an entity has a component, type or tag.

`type` can be an integer or string

#### `ecs.add(id, type)`

Add a component, type or tag to an entity.

`type` can be an integer or string

#### `ecs.remove(id, type)`

Remove a component, type or tag from an entity.

`type` can be an integer or string

#### `ecs.array(name, desc)`

Create an array component.

`desc` format: `"(type,N)"`

Returns the component entity

#### `ecs.struct(name, desc)`

Create a struct component.

`desc` format: `"{type member; ...}"`

Returns the component entity

#### `ecs.alias(name, alias)`

Create an `alias` of `name`.

Returns the component entity

#### `ecs.system(module, name, phase, [signature : string])` - DRAFT

Create a system in a new state

`module` Lua module name
`name` System function name
`phase` tag entity
`signature` System signature
