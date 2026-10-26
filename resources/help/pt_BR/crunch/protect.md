# Proteger uma planilha ou a pasta de trabalho
<!-- keywords: proteger, proteção, senha, bloquear, células bloqueadas, desbloquear, desproteger, estrutura, planilhas, somente leitura, permitir, formatar células, inserir linhas, classificar, filtrar, hash -->

## Proteger uma planilha

**Revisão > Proteger Planilha** impede que a planilha seja alterada por engano. Em uma planilha protegida, as células **bloqueadas** não podem ser alteradas. Todas as células ficam bloqueadas até que você indique o contrário: selecione as células que podem ser alteradas, abra **Formatar Células > Proteção** e desmarque **Bloqueada**. (Isso só tem efeito depois que a planilha é protegida.)

A caixa de diálogo pergunta o que todos ainda podem fazer na planilha: selecionar células bloqueadas ou desbloqueadas, formatar células, alterar colunas ou linhas, inserir ou excluir linhas e colunas, inserir links, classificar, usar filtros e alterar objetos de desenho e anotações. Tudo o que você deixar desmarcado será recusado, com uma mensagem que informa o que foi recusado.

A **senha** é opcional. Se você definir uma, a planilha só poderá ser desprotegida com ela. O JeffCrunch guarda uma forma embaralhada da senha (um hash com salt, repetido 100.000 vezes) e nunca a senha em si; por isso, ele não consegue mostrar a senha depois: guarde-a em um lugar seguro. Senhas criadas por outros programas, tanto no formato mais antigo quanto no mais recente, são verificadas da mesma forma.

Proteger e desproteger são ações que podem ser desfeitas. Para remover a proteção, use **Desproteger Planilha** e digite a senha, se houver.

Algumas ações continuam proibidas em uma planilha protegida, independentemente do que você permitir: mesclar células e excluir linhas ou colunas que contenham células bloqueadas.

## Proteger a pasta de trabalho

**Revisão > Proteger Pasta de Trabalho** bloqueia a **estrutura**: nenhuma planilha pode ser adicionada, excluída, renomeada, movida, ocultada ou reexibida. Também é possível exigir que as janelas sejam mantidas como estão. A proteção também pode ter uma senha. **Desproteger Pasta de Trabalho** remove a proteção.
