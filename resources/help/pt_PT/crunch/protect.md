# Proteger uma folha ou o livro
<!-- keywords: proteger, proteção, palavra-passe, bloquear, células bloqueadas, desbloquear, remover proteção, estrutura, folhas, só de leitura, permitir, formatar células, inserir linhas, ordenar, filtrar, hash, permitir intervalos de edição, intervalo editável, palavra-passe do intervalo -->

## Proteger uma folha

**Rever > Proteger Folha** evita que as pessoas alterem a folha por acidente. Numa folha protegida, as células que estão **bloqueadas** não podem ser alteradas. Todas as células estão bloqueadas até indicar o contrário: selecione as células que podem ser alteradas, abra **Formatar Células > Proteção** e desmarque **Bloqueada**. (Isto só tem efeito depois de a folha ser protegida.)

A caixa de diálogo pergunta o que todos podem continuar a fazer na folha: selecionar células bloqueadas ou desbloqueadas, formatar células, alterar colunas ou linhas, inserir ou eliminar linhas e colunas, introduzir ligações, ordenar, utilizar filtros, e alterar objetos de desenho e notas. Tudo o que deixar desmarcado é recusado, com uma mensagem que indica o que foi recusado.

A **palavra-passe** é opcional. Se a definir, a folha só pode ser desprotegida com essa palavra-passe. O JeffCrunch guarda uma forma codificada da palavra-passe (um hash com sal, repetido 100 000 vezes) e nunca a própria palavra-passe, por isso não lhe pode mostrar a palavra-passe mais tarde: guarde-a num local seguro. As palavras-passe criadas por outros programas, na forma antiga ou na nova, são verificadas da mesma maneira.

Proteger e desproteger são passos que pode anular. Para retirar a proteção, utilize **Desproteger Folha** e escreva a palavra-passe, se existir uma.

Algumas ações continuam proibidas numa folha protegida, qualquer que seja a permissão concedida: unir células e eliminar linhas ou colunas que contenham células bloqueadas.

## Permitir a alteração de algumas células: Permitir Intervalos de Edição

**Rever > Permitir Intervalos de Edição** lista os **intervalos** que as pessoas podem alterar, mesmo quando a folha está protegida. Um intervalo é um bloco de células, como A1:C5. Clique em **Novo** para acrescentar um. Dê-lhe um **título**, escreva as células que abrange (use um espaço entre blocos) e, se quiser, uma **palavra-passe**. Um intervalo sem palavra-passe pode ser alterado por qualquer pessoa. Um intervalo com palavra-passe só pode ser alterado depois de alguém escrever a palavra-passe na primeira vez que tenta alterar uma das suas células. Depois disso, o intervalo fica aberto até o livro ser fechado.

**Modificar** altera um intervalo, e **Eliminar** remove-o. Pode configurar os intervalos antes de proteger a folha. Ficam guardados com o livro, e as palavras-passe são mantidas na mesma forma codificada da palavra-passe da folha. Permitir Intervalos de Edição é um passo que pode anular. Quando insere ou elimina linhas, colunas ou células, os intervalos deslocam-se com as respetivas células, ficam maiores ou mais pequenos, ou desaparecem se todas as suas células forem eliminadas. Anular repõe-nos.

## Proteger o livro

**Rever > Proteger Livro** bloqueia a **estrutura**: não é possível adicionar, eliminar, mudar o nome, mover, ocultar ou mostrar nenhuma folha. Também pode exigir que as janelas sejam mantidas como estão. Pode ter igualmente uma palavra-passe. **Desproteger Livro** retira a proteção.
