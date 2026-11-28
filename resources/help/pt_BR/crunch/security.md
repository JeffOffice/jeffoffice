# Segurança: até onde um arquivo pode acessar
<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, vínculos, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, vínculos externos, programas, seguro -->

Um arquivo pode apontar para outras coisas: arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffCrunch faz com isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

Essas escolhas valem para todos os aplicativos do JeffOffice: se você alterar uma delas em um aplicativo, ela muda em todos. Uma linha referente a algo que um aplicativo não possui indica **Isto não se aplica** e aparece em cinza.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffCrunch as mantém no arquivo, de modo que continuam ali ao salvar, e **nunca as executa**. Não há nenhuma configuração que as execute.

## Arquivos vinculados e vínculos de pastas de trabalho

Trata-se dos outros arquivos que uma pasta de trabalho lê, como as pastas de trabalho de onde ela obtém números, chamadas **vínculos**, os arquivos que suas consultas leem e os arquivos a partir dos quais suas tabelas dinâmicas são criadas. A escolha define o que o JeffCrunch faz ao abrir uma pasta de trabalho que lê alguns desses arquivos e quando você usa **Atualizar** ou **Atualizar Tudo** em suas consultas e tabelas dinâmicas.

- **Perguntar antes** é a opção habitual. Antes que o JeffCrunch leia um desses arquivos, ele mostra o caminho completo do arquivo e pergunta. Clique em **Ler o Arquivo** somente se você confiar na origem da pasta de trabalho. **Não Ler**, o botão padrão, não lê nada e deixa a pasta de trabalho como estava. Esta opção pergunta toda vez que você abre uma pasta de trabalho e, depois, uma vez para cada arquivo. Depois que você clica em **Ler o Arquivo** para um arquivo, o JeffCrunch não pergunta novamente sobre esse mesmo arquivo nessa janela, até você fechá-la.
- **Atualizar** lê um arquivo sem perguntar somente se você já tiver autorizado essa mesma pasta de trabalho a ler esse mesmo arquivo. Para todos os outros arquivos, inclusive qualquer arquivo citado em uma pasta de trabalho que alguém acabou de enviar a você, ele ainda pergunta primeiro, da mesma forma.
- **Nunca atualizar** não lê nada quando uma pasta de trabalho é aberta e não mostra nenhuma pergunta. Os números que o arquivo armazenou permanecem como estão. Quando você clica em **Atualizar** por conta própria, isso é um pedido, e o JeffCrunch mostra a pergunta nesse momento. Depois que você clica em **Ler o Arquivo**, ele não pergunta novamente sobre esse mesmo arquivo nessa janela.

## O que fica lembrado

Quando você clica em **Ler o Arquivo** ou **Ler os Arquivos**, o JeffCrunch lembra desse par: esta pasta de trabalho e aquele arquivo, cada um pelo seu caminho completo. Somente o par é lembrado. Se a pasta de trabalho for movida ou renomeada, editada para ler outro arquivo, ou se o arquivo for substituído por um atalho para outro arquivo, o JeffCrunch perguntará novamente. Uma pasta de trabalho que você nunca salvou não tem caminho, de modo que nada sobre ela é lembrado depois que você a fecha.

Os pares ficam guardados apenas nas configurações do seu próprio computador, nunca em uma pasta de trabalho, de modo que uma pasta de trabalho não pode acrescentar um par, e ninguém que receba uma pasta de trabalho sua obtém esses pares. As configurações guardam uma forma embaralhada de cada caminho, e não o caminho em si. São mantidos até 2.000 pares; quando há mais, o par usado há mais tempo é esquecido primeiro.

**Opções > Segurança** mostra quantos pares estão lembrados (**Autorizações lembradas**). Clique em **Esquecer Tudo** para esquecê-los todos de uma vez. Nenhuma pasta de trabalho é alterada, e cada uma delas perguntará novamente na próxima vez.

Uma pasta de trabalho que diz "nunca" em seu próprio **Aviso de inicialização** (**Dados > Editar Vínculos**) é sempre deixada de lado. Uma pasta de trabalho de outro computador nunca é lida quando um arquivo é aberto, qualquer que seja a sua escolha. Você ainda pode usar **Atualizar Valores** em **Editar Vínculos** por conta própria a qualquer momento.

## Filmes e sons vinculados

Isto não se aplica no JeffCrunch: ele não tem filmes nem sons vinculados para reproduzir. A linha aparece em cinza.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta escolha define como o JeffCrunch o abre.

- **Abrir como somente leitura primeiro** é a opção habitual. O arquivo abre para que você possa lê-lo, mas não pode alterá-lo. Uma barra sob o arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar somente esse arquivo (até você fechar o programa). Quando você o salvar no mesmo lugar depois, a marca de download é removida do arquivo. Ou use **Salvar Como** para criar uma cópia, ou altere esta configuração para a próxima opção.
- **Abrir normalmente** abre todos os arquivos da mesma forma.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Somente Leitura** a partir da tela **Arquivo** abre desse modo, independentemente do que esta opção indicar.

## Vínculos que iniciam um programa

Um vínculo que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e similares) é **sempre recusado**. O JeffCrunch avisa você, e nada é executado. Um vínculo para um arquivo em outro computador nunca é seguido sem que você seja consultado antes. Nenhuma configuração altera isso.

## Veja também

- [Alterar as configurações do JeffCrunch](options)
- [Notas e links](notes-links)
