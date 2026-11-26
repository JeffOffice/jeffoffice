# Proteger um documento e bloquear o controle de alterações
<!-- keywords: proteger, proteção, restringir edição, somente leitura, senha, bloquear controle de alterações, opções de controle de alterações, nome de usuário, iniciais, somente alterações controladas, somente comentários, parar proteção -->

A proteção limita o que as pessoas podem fazer em um documento. Estas ferramentas estão na guia **Revisão**.

## Restringir edição

1. Clique em **Restringir Edição** no grupo **Proteger**.
2. Escolha o que as pessoas podem fazer:
   - **Nenhuma alteração (somente leitura)**: o texto não pode ser alterado.
   - **Alterações controladas**: as pessoas podem editar, mas toda edição é controlada, e o Controle de Alterações não pode ser desativado.
   - **Comentários**: as pessoas podem adicionar comentários, mas não podem alterar o texto.
   - **Preenchimento de formulários**: o texto é somente leitura, mas as pessoas podem preencher os campos de formulário que ele contém.
3. Digite uma senha, duas vezes. Você pode deixar a senha em branco. Nesse caso, qualquer pessoa pode interromper a proteção.
4. Clique em **Iniciar Aplicação**.

A barra de título passa a dizer "Protegido". Para interromper, clique em **Restringir Edição** novamente e, em seguida, em **Interromper Proteção**. Digite a senha, se houver uma.

A senha evita que pessoas de boa-fé alterem o documento por engano. Ela não é uma segurança forte. Não use a proteção para esconder informações privadas.

A senha é guardada no arquivo como um número embaralhado, chamado de hash. A própria senha nunca é salva. Se você a esquecer, o JeffWrite não consegue recuperá-la.

## Preencher um formulário

Um documento pode ter campos de formulário: caixas para digitar, caixas de seleção e listas para escolher. O JeffWrite preenche esses campos, mas não adiciona novos.

- **Digitar em um campo.** Clique nele e digite. Um campo pode aceitar apenas uma quantidade limitada de letras, converter o que você digita em maiúsculas, ou aceitar apenas um número ou uma data. O JeffWrite respeita isso ao digitar, colar ou inserir texto com um método de entrada. Texto colado que é cortado para caber mantém suas primeiras letras. Uma quebra de linha no texto colado vira um espaço. Uma quebra de linha que já está no campo permanece, a menos que você a selecione e digite por cima dela; digitar nunca acrescenta uma. Um número ou uma data é formatado no formato definido quando você sai do campo: pressione Tab, pressione Enter, clique em outro lugar, saia com as teclas de seta, salve ou imprima.
- **Marcar uma caixa de seleção.** Clique nela, ou mova até ela e pressione a Barra de Espaço.
- **Escolher em uma lista.** Clique nela, ou mova até ela e pressione Enter. Depois escolha uma entrada. Ao pressionar uma letra, é selecionada a próxima entrada que começa com ela.
- **Mover-se entre os campos.** Quando o documento está protegido para preenchimento de formulários, pressione Tab ou Enter para ir ao próximo campo, e Shift+Tab para voltar. Em qualquer documento, Enter dentro de um campo não inicia um novo parágrafo. Somente os campos podem ser alterados. Uma parte do documento que quem criou o formulário deixou fora da proteção também pode ser alterada; nela, Tab é a tecla comum, e Substituir Tudo altera apenas as ocorrências nessa parte, informa quantas deixou de fora e diz que o texto em cabeçalhos, rodapés, notas e caixas de texto foi mantido como está. O que você digita, cola, arrasta ou corrige é verificado pelo local onde cairia, e não pela posição do cursor. Imagens, formas e tabelas podem ser inseridas e alteradas em uma parte que ficou fora da proteção, como em qualquer documento, e não são alteradas em nenhum outro lugar.
- **Desfazer.** Pressione Desfazer. As letras que você digita em um campo formam uma etapa. Desfazer retorna as etapas feitas desde o início da proteção, que são suas entradas e suas alterações em uma parte fora da proteção, e nada de antes.

Um campo desativado no formulário, ou um campo que o próprio formulário calcula, não pode ser preenchido. Em qualquer documento, as marcas de início, de fim e outras marcas de um campo nunca são excluídas uma a uma: selecione o campo inteiro para excluí-lo. Um campo em um documento somente leitura, ou aberto apenas para comentários, também não pode ser preenchido. Com o Controle de Alterações ativado em um documento que não está protegido, o que você digita em um campo de texto é uma alteração controlada, e o tamanho máximo do texto e as maiúsculas do campo não são aplicados.

## Bloquear o controle de alterações

Clique em **Opções de Controle de Alterações**, depois em **Bloquear Controle de Alterações**. Todas as alterações feitas a partir de então são controladas. Ninguém pode desativar o Controle de Alterações, nem aceitar ou rejeitar uma alteração, sem a senha. É o mesmo que proteger o documento para alterações controladas.

## Alterar seu nome nas alterações

Clique em **Opções de Controle de Alterações** e, em seguida, em **Alterar Opções de Controle de Alterações**. Digite seu nome e suas iniciais. Eles assinam as alterações e os comentários que você fizer a partir de então.
