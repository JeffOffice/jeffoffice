# Proteger uma planilha ou a pasta de trabalho
<!-- keywords: proteger, proteção, senha, bloquear, células bloqueadas, desbloquear, desproteger, estrutura, planilhas, somente leitura, permitir, formatar células, inserir linhas, classificar, filtrar, hash, permitir intervalos de edição, intervalo editável, senha do intervalo -->

## Proteger uma planilha

**Revisão > Proteger Planilha** impede que as pessoas alterem a planilha por acidente. Em uma planilha protegida, as células que estão **bloqueadas** não podem ser alteradas. Todas as células ficam bloqueadas até que você decida o contrário: selecione as células que podem ser alteradas, abra **Formatar Células > Proteção** e desmarque **Bloqueada**. (Isso só faz diferença depois que a planilha estiver protegida.)

A caixa de diálogo pergunta o que todos ainda podem fazer na planilha: selecionar células bloqueadas ou desbloqueadas, formatar células, alterar colunas ou linhas, inserir ou excluir linhas e colunas, inserir links, classificar, usar filtros e alterar objetos de desenho e anotações. O que você deixar desmarcado será recusado, com uma mensagem que informa o que foi recusado.

Uma **senha** é opcional. Se você definir uma, a planilha só poderá ser desprotegida com ela. O JeffCrunch guarda uma forma embaralhada da senha (um hash com sal, repetido 100.000 vezes) e nunca a senha em si, por isso não consegue mostrá-la depois: guarde-a em local seguro. Senhas criadas por outros programas, na forma antiga ou na nova, são verificadas da mesma maneira.

Proteger e desproteger são etapas que você pode desfazer. Para remover a proteção, use **Desproteger Planilha** e digite a senha, se houver uma.

Algumas ações continuam não permitidas em uma planilha protegida, independentemente do que você liberar: mesclar células e excluir linhas ou colunas que contenham células bloqueadas.

## Permitir a alteração de algumas células: Permitir Intervalos de Edição

**Revisão > Permitir Intervalos de Edição** lista os **intervalos** que as pessoas podem alterar mesmo quando a planilha está protegida. Um intervalo é um bloco de células, como A1:C5. Clique em **Novo** para adicionar um. Dê a ele um **título**, digite as células que ele abrange (use um espaço entre os blocos) e, se quiser, uma **senha**. Um intervalo sem senha pode ser alterado por qualquer pessoa. Um intervalo com senha só pode ser alterado depois que alguém digitar a senha na primeira vez em que tentar alterar uma de suas células. Depois disso, o intervalo permanece liberado até que a pasta de trabalho seja fechada.

**Modificar** altera um intervalo, e **Excluir** o remove. Você pode configurar os intervalos antes de proteger a planilha. Eles são salvos com a pasta de trabalho, e as senhas são mantidas na mesma forma embaralhada da senha da planilha. Permitir Intervalos de Edição é uma etapa que você pode desfazer. Ao inserir ou excluir linhas, colunas ou células, os intervalos se movem com as suas células, ficam menores ou maiores, ou desaparecem se todas as suas células forem excluídas. Desfazer os traz de volta.

## Proteger a pasta de trabalho

**Revisão > Proteger Pasta de Trabalho** bloqueia a **estrutura**: nenhuma planilha pode ser adicionada, excluída, renomeada, movida, ocultada ou reexibida. Também é possível pedir que as janelas sejam mantidas como estão. A proteção pode ter uma senha também. **Desproteger Pasta de Trabalho** a remove.
